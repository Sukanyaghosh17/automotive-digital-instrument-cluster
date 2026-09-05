#ifndef VEHICLE_STATUS_MODEL_HPP
#define VEHICLE_STATUS_MODEL_HPP

#include <cstdint>

// Warning flags bitmask definitions
constexpr uint8_t WARNING_FLAG_ENGINE_OVERHEAT = 0x01; // Bit 0
constexpr uint8_t WARNING_FLAG_LOW_FUEL        = 0x02; // Bit 1
constexpr uint8_t WARNING_FLAG_ABS_FAULT       = 0x04; // Bit 2

class VehicleStatusModel
{
public:
    VehicleStatusModel();

    void update(uint16_t currentSpeed, uint16_t currentRpm);

    // Manual overrides for testing/scenarios
    void setFuel(uint8_t fuel) { m_fuel = fuel; }
    void setEngineTemp(int16_t temp) { m_engineTemp = temp; }
    void toggleAbsFault() { m_absFault = !m_absFault; }

    uint8_t getFuel() const { return m_fuel; }
    int16_t getEngineTemp() const { return m_engineTemp; }
    uint8_t getWarningFlags() const;

    bool isOverheat() const { return m_engineTemp > 110; }
    bool isLowFuel() const { return m_fuel < 15; }
    bool isAbsFault() const { return m_absFault; }

private:
    uint8_t m_fuel;
    int16_t m_engineTemp;
    bool m_absFault;
    uint32_t m_tickCounter;
};

#endif // VEHICLE_STATUS_MODEL_HPP
