#ifndef CAN_TRANSMITTER_HPP
#define CAN_TRANSMITTER_HPP

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <string>

// CAN Frame IDs per approved specification
constexpr uint32_t CAN_ID_POWERTRAIN     = 0x100;
constexpr uint32_t CAN_ID_VEHICLE_STATUS = 0x200;
constexpr uint32_t CAN_ID_WARNINGS       = 0x300;

constexpr uint8_t CAN_STANDARD_DLC = 8;

class CANTransmitter
{
public:
    explicit CANTransmitter(const std::string& interfaceName = "vcan0");
    ~CANTransmitter();

    bool init();
    bool sendFrame(uint32_t canId, const uint8_t* data, uint8_t dlc = CAN_STANDARD_DLC);
    void closeSocket();

    bool isConnected() const { return m_socketFd >= 0; }
    std::string getInterfaceName() const { return m_interfaceName; }

private:
    std::string m_interfaceName;
    int m_socketFd;
    struct sockaddr_can m_addr;
};

#endif // CAN_TRANSMITTER_HPP
