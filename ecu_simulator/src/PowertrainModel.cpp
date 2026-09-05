#include "PowertrainModel.hpp"
#include <algorithm>

PowertrainModel::PowertrainModel()
    : m_speed(0),
      m_rpm(RPM_IDLE),
      m_gear(Gear::P),
      m_driveMode(DriveMode::Comfort)
{
}

void PowertrainModel::accelerate()
{
    // Auto-engage Drive if accelerating from standstill and in Park or Neutral
    if (m_gear == Gear::P || m_gear == Gear::N)
    {
        m_gear = Gear::D;
    }

    uint16_t speedStep = 4;
    switch (m_driveMode)
    {
        case DriveMode::Eco:     speedStep = 3; break;
        case DriveMode::Comfort: speedStep = 5; break;
        case DriveMode::Sport:   speedStep = 8; break;
    }

    if (m_gear == Gear::D)
    {
        if (m_speed + speedStep <= SPEED_MAX)
            m_speed += speedStep;
        else
            m_speed = SPEED_MAX;
    }
    else if (m_gear == Gear::R)
    {
        // Reverse max speed limited to 35 km/h
        constexpr uint16_t REVERSE_MAX_SPEED = 35;
        if (m_speed + 2 <= REVERSE_MAX_SPEED)
            m_speed += 2;
        else
            m_speed = REVERSE_MAX_SPEED;
    }

    updateRpm();
}

void PowertrainModel::brake()
{
    uint16_t brakeStep = 8;
    if (m_speed >= brakeStep)
    {
        m_speed -= brakeStep;
    }
    else
    {
        m_speed = 0;
    }

    updateRpm();
}

void PowertrainModel::idleDecay()
{
    if (m_speed > 0)
    {
        m_speed = (m_speed > 1) ? (m_speed - 1) : 0;
    }
    updateRpm();
}

void PowertrainModel::cycleDriveMode()
{
    switch (m_driveMode)
    {
        case DriveMode::Eco:     m_driveMode = DriveMode::Comfort; break;
        case DriveMode::Comfort: m_driveMode = DriveMode::Sport;   break;
        case DriveMode::Sport:   m_driveMode = DriveMode::Eco;     break;
    }
    updateRpm();
}

bool PowertrainModel::setGear(Gear newGear)
{
    // State machine enforcement:
    // - P and N only reachable at standstill (speed == 0)
    // - R only reachable at standstill, and only transitions back to P, N, or standstill D
    // - D allowed whenever speed > 0
    if (newGear == m_gear)
    {
        return true;
    }

    if (newGear == Gear::P || newGear == Gear::N || newGear == Gear::R)
    {
        if (m_speed > 0)
        {
            // Disallow shifting to P, N, or R while moving
            return false;
        }
    }

    m_gear = newGear;
    updateRpm();
    return true;
}

void PowertrainModel::updateRpm()
{
    if (m_gear == Gear::P || m_gear == Gear::N)
    {
        m_rpm = RPM_IDLE;
        return;
    }

    // Realistic multi-gear shift simulation based on speed
    // 6-speed transmission simulation
    float ratio = 1.0f;
    if (m_speed < 25)
        ratio = 120.0f; // 1st gear
    else if (m_speed < 50)
        ratio = 75.0f;  // 2nd gear
    else if (m_speed < 80)
        ratio = 52.0f;  // 3rd gear
    else if (m_speed < 120)
        ratio = 38.0f;  // 4th gear
    else if (m_speed < 160)
        ratio = 28.0f;  // 5th gear
    else
        ratio = 22.0f;  // 6th gear

    // Drive mode RPM aggressiveness multiplier
    float modeMultiplier = 1.0f;
    if (m_driveMode == DriveMode::Eco)
        modeMultiplier = 0.85f;
    else if (m_driveMode == DriveMode::Sport)
        modeMultiplier = 1.25f;

    uint32_t calculated = RPM_IDLE + static_cast<uint32_t>((m_speed % 40 + 10) * ratio * modeMultiplier / 10.0f);

    if (m_speed == 0)
    {
        m_rpm = RPM_IDLE;
    }
    else
    {
        m_rpm = static_cast<uint16_t>(std::min<uint32_t>(calculated, RPM_MAX));
    }
}

std::string PowertrainModel::getGearString() const
{
    switch (m_gear)
    {
        case Gear::P: return "P";
        case Gear::R: return "R";
        case Gear::N: return "N";
        case Gear::D: return "D";
    }
    return "P";
}

std::string PowertrainModel::getDriveModeString() const
{
    switch (m_driveMode)
    {
        case DriveMode::Eco:     return "ECO";
        case DriveMode::Comfort: return "COMFORT";
        case DriveMode::Sport:   return "SPORT";
    }
    return "COMFORT";
}
