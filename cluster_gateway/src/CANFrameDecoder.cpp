#include "CANFrameDecoder.hpp"

bool CANFrameDecoder::decodePowertrain(const struct can_frame& frame, PowertrainData& outData)
{
    // CAN ID 0x100 (Powertrain_Frame) - Big-Endian
    // Bytes 0-1: Speed (uint16)
    // Bytes 2-3: RPM (uint16)
    // Byte 4:    Gear (uint8)
    // Byte 5:    Drive Mode (uint8)
    if (frame.can_id != 0x100 || frame.can_dlc < 2)
    {
        return false;
    }

    outData.speed = (static_cast<uint16_t>(frame.data[0]) << 8) | static_cast<uint16_t>(frame.data[1]);
    outData.rpm = (frame.can_dlc >= 4) ? ((static_cast<uint16_t>(frame.data[2]) << 8) | static_cast<uint16_t>(frame.data[3])) : 0;
    outData.gear = (frame.can_dlc >= 5) ? frame.data[4] : 0;
    outData.driveMode = (frame.can_dlc >= 6) ? frame.data[5] : 1; // Default to Comfort

    return true;
}

bool CANFrameDecoder::decodeVehicleStatus(const struct can_frame& frame, VehicleStatusData& outData)
{
    // CAN ID 0x200 (VehicleStatus_Frame) - Big-Endian
    // Byte 0:   Fuel (uint8)
    // Byte 1-2: Engine coolant temp (int16_t, -40 to +150°C)
    if (frame.can_id != 0x200 || frame.can_dlc < 1)
    {
        return false;
    }

    outData.fuelLevel = frame.data[0];

    if (frame.can_dlc >= 3)
    {
        uint16_t rawTemp = (static_cast<uint16_t>(frame.data[1]) << 8) | static_cast<uint16_t>(frame.data[2]);
        outData.engineTemp = static_cast<int16_t>(rawTemp);
    }
    else
    {
        outData.engineTemp = 90;
    }

    return true;
}

bool CANFrameDecoder::decodeWarnings(const struct can_frame& frame, WarningData& outData)
{
    // CAN ID 0x300 (Warning_Frame)
    // Byte 0: Warning bitmask
    if (frame.can_id != 0x300 || frame.can_dlc < 1)
    {
        return false;
    }

    outData.warningFlags = frame.data[0];
    return true;
}
