#include "ClusterGateway.hpp"
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <vector>

ClusterGateway::ClusterGateway(const std::string& canInterface)
    : m_canInterface(canInterface),
      m_socketFd(-1),
      m_running(false),
      m_app(nullptr)
{
}

ClusterGateway::~ClusterGateway()
{
    stop();
    if (m_socketFd >= 0)
    {
        close(m_socketFd);
    }
}

void ClusterGateway::init()
{
    m_app = vsomeip::runtime::get()->create_application("ClusterGateway");
    if (!m_app)
    {
        throw std::runtime_error("[ClusterGateway] Failed to create vsomeip application");
    }

    if (!m_app->init())
    {
        throw std::runtime_error("[ClusterGateway] Failed to initialize vsomeip application");
    }

    // Register Eventgroup 0x0001
    std::set<vsomeip::eventgroup_t> eventGroup;
    eventGroup.insert(EVENTGROUP_ID);

    // Offer all three typed telemetry events BEFORE offering the service
    m_app->offer_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_POWERTRAIN, eventGroup);
    m_app->offer_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_VEHICLE_STATUS, eventGroup);
    m_app->offer_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_WARNINGS, eventGroup);

    // Offer Service 0x1234, Instance 0x5678
    m_app->offer_service(SERVICE_ID, INSTANCE_ID);

    setupCanSocket();
    std::cout << "[ClusterGateway] Initialized successfully. Service 0x" 
              << std::hex << SERVICE_ID << " offered." << std::dec << std::endl;
}

void ClusterGateway::start()
{
    m_running = true;
    m_canThread = std::thread(&ClusterGateway::canReaderThread, this);
    m_app->start();
}

void ClusterGateway::stop()
{
    m_running = false;
    if (m_canThread.joinable())
    {
        m_canThread.join();
    }
    if (m_app)
    {
        m_app->stop();
    }
}

void ClusterGateway::setupCanSocket()
{
    m_socketFd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (m_socketFd < 0)
    {
        throw std::runtime_error("[ClusterGateway] Error opening SocketCAN socket");
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, m_canInterface.c_str(), IFNAMSIZ - 1);

    if (ioctl(m_socketFd, SIOCGIFINDEX, &ifr) < 0)
    {
        close(m_socketFd);
        throw std::runtime_error("[ClusterGateway] Error retrieving index for interface: " + m_canInterface);
    }

    struct sockaddr_can addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(m_socketFd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        close(m_socketFd);
        throw std::runtime_error("[ClusterGateway] Error binding to CAN socket on " + m_canInterface);
    }
}

void ClusterGateway::canReaderThread()
{
    std::cout << "[ClusterGateway] Listening for CAN frames on " << m_canInterface << "..." << std::endl;

    while (m_running)
    {
        struct can_frame frame;
        ssize_t nbytes = read(m_socketFd, &frame, sizeof(struct can_frame));
        if (nbytes > 0)
        {
            if (frame.can_id == 0x100)
            {
                PowertrainData pt;
                if (CANFrameDecoder::decodePowertrain(frame, pt))
                {
                    publishPowertrain(pt);
                }
            }
            else if (frame.can_id == 0x200)
            {
                VehicleStatusData vs;
                if (CANFrameDecoder::decodeVehicleStatus(frame, vs))
                {
                    publishVehicleStatus(vs);
                }
            }
            else if (frame.can_id == 0x300)
            {
                WarningData wd;
                if (CANFrameDecoder::decodeWarnings(frame, wd))
                {
                    publishWarnings(wd);
                }
            }
        }
    }
}

// =========================================================================
// SOME/IP Payload Serialization (AUTOSAR Standard: Big-Endian)
// =========================================================================

void ClusterGateway::publishPowertrain(const PowertrainData& data)
{
    // PowertrainEvent (0x8001) - 6 Bytes:
    //   Bytes 0-1: Speed (uint16_t MSB, LSB)
    //   Bytes 2-3: RPM   (uint16_t MSB, LSB)
    //   Byte 4:    Gear  (uint8_t enum)
    //   Byte 5:    Drive Mode (uint8_t enum)
    std::vector<vsomeip::byte_t> payloadData(6, 0);
    payloadData[0] = static_cast<vsomeip::byte_t>((data.speed >> 8) & 0xFF);
    payloadData[1] = static_cast<vsomeip::byte_t>(data.speed & 0xFF);
    payloadData[2] = static_cast<vsomeip::byte_t>((data.rpm >> 8) & 0xFF);
    payloadData[3] = static_cast<vsomeip::byte_t>(data.rpm & 0xFF);
    payloadData[4] = static_cast<vsomeip::byte_t>(data.gear);
    payloadData[5] = static_cast<vsomeip::byte_t>(data.driveMode);

    auto payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(payloadData);
    m_app->notify(SERVICE_ID, INSTANCE_ID, EVENT_ID_POWERTRAIN, payload);
}

void ClusterGateway::publishVehicleStatus(const VehicleStatusData& data)
{
    // VehicleStatusEvent (0x8002) - 3 Bytes:
    //   Byte 0:    Fuel Level (uint8_t)
    //   Bytes 1-2: Engine Coolant Temp (int16_t MSB, LSB)
    std::vector<vsomeip::byte_t> payloadData(3, 0);
    payloadData[0] = static_cast<vsomeip::byte_t>(data.fuelLevel);

    uint16_t rawTemp = static_cast<uint16_t>(data.engineTemp);
    payloadData[1] = static_cast<vsomeip::byte_t>((rawTemp >> 8) & 0xFF);
    payloadData[2] = static_cast<vsomeip::byte_t>(rawTemp & 0xFF);

    auto payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(payloadData);
    m_app->notify(SERVICE_ID, INSTANCE_ID, EVENT_ID_VEHICLE_STATUS, payload);
}

void ClusterGateway::publishWarnings(const WarningData& data)
{
    // WarningEvent (0x8003) - 1 Byte:
    //   Byte 0: Warning flags bitmask
    std::vector<vsomeip::byte_t> payloadData(1, data.warningFlags);

    auto payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(payloadData);
    m_app->notify(SERVICE_ID, INSTANCE_ID, EVENT_ID_WARNINGS, payload);
}
