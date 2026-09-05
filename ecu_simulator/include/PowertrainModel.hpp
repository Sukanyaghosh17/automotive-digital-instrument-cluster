#ifndef POWERTRAIN_MODEL_HPP
#define POWERTRAIN_MODEL_HPP

#include <cstdint>
#include <string>

enum class Gear : uint8_t
{
    P = 0,
    R = 1,
    N = 2,
    D = 3
};

enum class DriveMode : uint8_t
{
    Eco = 0,
    Comfort = 1,
    Sport = 2
};

class PowertrainModel
{
public:
    PowertrainModel();

    void accelerate();
    void brake();
    void idleDecay();
    void cycleDriveMode();
    bool setGear(Gear newGear);

    uint16_t getSpeed() const { return m_speed; }
    uint16_t getRpm() const { return m_rpm; }
    Gear getGear() const { return m_gear; }
    DriveMode getDriveMode() const { return m_driveMode; }

    std::string getGearString() const;
    std::string getDriveModeString() const;

private:
    void updateRpm();

    uint16_t m_speed;
    uint16_t m_rpm;
    Gear m_gear;
    DriveMode m_driveMode;

    static constexpr uint16_t SPEED_MIN = 0U;
    static constexpr uint16_t SPEED_MAX = 260U;
    static constexpr uint16_t RPM_IDLE  = 800U;
    static constexpr uint16_t RPM_MAX   = 8000U;
};

#endif // POWERTRAIN_MODEL_HPP
