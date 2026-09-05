#ifndef CLUSTER_GATEWAY_HPP
#define CLUSTER_GATEWAY_HPP

#include "CANFrameDecoder.hpp"
#include <vsomeip/vsomeip.hpp>
#include <thread>
#include <atomic>
#include <string>

class ClusterGateway
{
public:
    explicit ClusterGateway(const std::string& canInterface = "vcan0");
    ~ClusterGateway();

    void init();
    void start();
    void stop();

    void publishPowertrain(const PowertrainData& data);
    void publishVehicleStatus(const VehicleStatusData& data);
    void publishWarnings(const WarningData& data);

    // SOME/IP Service Constants
    static constexpr vsomeip::service_t    SERVICE_ID               = 0x1234;
    static constexpr vsomeip::instance_t   INSTANCE_ID              = 0x5678;
    static constexpr vsomeip::eventgroup_t EVENTGROUP_ID            = 0x0001;

    static constexpr vsomeip::event_t      EVENT_ID_POWERTRAIN      = 0x8001;
    static constexpr vsomeip::event_t      EVENT_ID_VEHICLE_STATUS  = 0x8002;
    static constexpr vsomeip::event_t      EVENT_ID_WARNINGS        = 0x8003;

private:
    void canReaderThread();
    void setupCanSocket();

    std::string m_canInterface;
    int m_socketFd;
    std::atomic<bool> m_running;
    std::thread m_canThread;
    std::shared_ptr<vsomeip::application> m_app;
};

#endif // CLUSTER_GATEWAY_HPP
