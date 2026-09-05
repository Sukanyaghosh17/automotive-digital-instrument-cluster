#include "VehicleStatusModel.hpp"
#include <algorithm>

VehicleStatusModel::VehicleStatusModel()
    : m_fuel(85),
      m_engineTemp(75),
      m_absFault(false),
      m_tickCounter(0)
{
}

void VehicleStatusModel::update(uint16_t currentSpeed, uint16_t currentRpm)
{
    m_tickCounter++;

    // Gradual engine warmup towards nominal operating temperature (90°C)
    // If driving aggressively at high RPM (> 5000), temperature climbs higher
    if (m_tickCounter % 30 == 0)
    {
        if (currentRpm > 5500)
        {
            if (m_engineTemp < 115) // Can reach stress-test overheat under sustained high revs
                m_engineTemp += 1;
        }
        else if (m_engineTemp < 90)
        {
            m_engineTemp += 1;
        }
        else if (m_engineTemp > 90 && currentRpm < 3000)
        {
            m_engineTemp -= 1; // Cools back down to 90
        }
    }

    // Fuel depletion simulation based on distance/speed
    // Depletes by 1% periodically while driving
    if (m_tickCounter % 150 == 0 && currentSpeed > 0)
    {
        if (m_fuel > 0)
            m_fuel -= 1;
    }
}

uint8_t VehicleStatusModel::getWarningFlags() const
{
    uint8_t flags = 0x00;

    if (m_engineTemp > 110)
    {
        flags |= WARNING_FLAG_ENGINE_OVERHEAT; // Bit 0
    }

    if (m_fuel < 15)
    {
        flags |= WARNING_FLAG_LOW_FUEL;        // Bit 1
    }

    if (m_absFault)
    {
        flags |= WARNING_FLAG_ABS_FAULT;       // Bit 2
    }

    return flags;
}
