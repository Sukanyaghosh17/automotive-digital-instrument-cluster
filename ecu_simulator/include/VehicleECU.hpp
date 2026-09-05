#ifndef VEHICLE_ECU_HPP
#define VEHICLE_ECU_HPP

#include "PowertrainModel.hpp"
#include "VehicleStatusModel.hpp"
#include "CANTransmitter.hpp"
#include <string>

class VehicleECU
{
public:
    explicit VehicleECU(const std::string& ifname = "vcan0");

    bool init();
    void step();

    // Controls
    void throttle();
    void brake();
    void idle();
    void cycleDriveMode();
    bool requestGear(Gear gear);
    void toggleAbsFault();
    void triggerOverheatTest();
    void triggerLowFuelTest();

    // Getters for UI/Console
    const PowertrainModel& getPowertrain() const { return m_powertrain; }
    const VehicleStatusModel& getStatus() const { return m_status; }

private:
    void broadcastPowertrainFrame();
    void broadcastVehicleStatusFrame();
    void broadcastWarningFrame();

    PowertrainModel m_powertrain;
    VehicleStatusModel m_status;
    CANTransmitter m_can;
    uint32_t m_cycleCounter;
};

#endif // VEHICLE_ECU_HPP
