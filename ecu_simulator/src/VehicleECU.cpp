#include "VehicleECU.hpp"
#include <iostream>
#include <cstring>

VehicleECU::VehicleECU(const std::string& ifname)
    : m_can(ifname), m_cycleCounter(0)
{
}

bool VehicleECU::init()
{
    return m_can.init();
}

void VehicleECU::throttle()
{
    m_powertrain.accelerate();
}

void VehicleECU::brake()
{
    m_powertrain.brake();
}

void VehicleECU::idle()
{
    m_powertrain.idleDecay();
}

void VehicleECU::cycleDriveMode()
{
    m_powertrain.cycleDriveMode();
}

bool VehicleECU::requestGear(Gear gear)
{
    return m_powertrain.setGear(gear);
}

void VehicleECU::toggleAbsFault()
{
    m_status.toggleAbsFault();
}

void VehicleECU::triggerOverheatTest()
{
    // Toggle between normal operating temp (90°C) and overheat stress-test (118°C)
    if (m_status.getEngineTemp() > 110)
        m_status.setEngineTemp(90);
    else
        m_status.setEngineTemp(118);
}

void VehicleECU::triggerLowFuelTest()
{
    // Toggle between normal fuel (65%) and low fuel warning (10%)
    if (m_status.getFuel() < 15)
        m_status.setFuel(65);
    else
        m_status.setFuel(10);
}

void VehicleECU::step()
{
    m_cycleCounter++;
    m_status.update(m_powertrain.getSpeed(), m_powertrain.getRpm());

    // CAN Frame 0x100: Powertrain (Sent every step: ~50-100ms)
    broadcastPowertrainFrame();

    // CAN Frame 0x200: Vehicle Status (Sent every 5 cycles: ~250-500ms)
    if (m_cycleCounter % 5 == 0)
    {
        broadcastVehicleStatusFrame();
    }

    // CAN Frame 0x300: Warnings (Sent every 2 cycles: ~100-200ms)
    if (m_cycleCounter % 2 == 0)
    {
        broadcastWarningFrame();
    }
}

void VehicleECU::broadcastPowertrainFrame()
{
    // CAN ID 0x100 (Powertrain_Frame - 8 Bytes, Big-Endian):
    // Byte 0-1: Speed (uint16_t)
    // Byte 2-3: RPM (uint16_t)
    // Byte 4:   Gear (uint8_t: 0=P, 1=R, 2=N, 3=D)
    // Byte 5:   Drive Mode (uint8_t: 0=Eco, 1=Comfort, 2=Sport)
    // Byte 6-7: Reserved (0x00, 0x00)
    uint8_t payload[8];
    std::memset(payload, 0, sizeof(payload));

    uint16_t speed = m_powertrain.getSpeed();
    uint16_t rpm   = m_powertrain.getRpm();
    uint8_t gear   = static_cast<uint8_t>(m_powertrain.getGear());
    uint8_t mode   = static_cast<uint8_t>(m_powertrain.getDriveMode());

    payload[0] = static_cast<uint8_t>((speed >> 8) & 0xFFU);
    payload[1] = static_cast<uint8_t>(speed & 0xFFU);
    payload[2] = static_cast<uint8_t>((rpm >> 8) & 0xFFU);
    payload[3] = static_cast<uint8_t>(rpm & 0xFFU);
    payload[4] = gear;
    payload[5] = mode;
    payload[6] = 0x00;
    payload[7] = 0x00;

    m_can.sendFrame(CAN_ID_POWERTRAIN, payload, 8);
}

void VehicleECU::broadcastVehicleStatusFrame()
{
    // CAN ID 0x200 (VehicleStatus_Frame - 8 Bytes):
    // Byte 0:   Fuel level (uint8_t, 0-100%)
    // Byte 1-2: Engine coolant temp (int16_t, Big-Endian, -40 to +150°C)
    // Byte 3-7: Reserved (0x00 padding)
    uint8_t payload[8];
    std::memset(payload, 0, sizeof(payload));

    uint8_t fuel      = m_status.getFuel();
    int16_t temp      = m_status.getEngineTemp();
    uint16_t tempRaw  = static_cast<uint16_t>(temp);

    payload[0] = fuel;
    payload[1] = static_cast<uint8_t>((tempRaw >> 8) & 0xFFU);
    payload[2] = static_cast<uint8_t>(tempRaw & 0xFFU);

    m_can.sendFrame(CAN_ID_VEHICLE_STATUS, payload, 8);
}

void VehicleECU::broadcastWarningFrame()
{
    // CAN ID 0x300 (Warning_Frame - 8 Bytes):
    // Byte 0:   Warning flags bitmask (Bit 0: Overheat, Bit 1: Low Fuel, Bit 2: ABS)
    // Byte 1-7: Reserved (0x00 padding)
    uint8_t payload[8];
    std::memset(payload, 0, sizeof(payload));

    payload[0] = m_status.getWarningFlags();

    m_can.sendFrame(CAN_ID_WARNINGS, payload, 8);
}
