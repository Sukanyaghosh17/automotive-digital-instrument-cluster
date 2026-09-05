#include "CANTransmitter.hpp"
#include <iostream>

CANTransmitter::CANTransmitter(const std::string& interfaceName)
    : m_interfaceName(interfaceName), m_socketFd(-1)
{
    std::memset(&m_addr, 0, sizeof(m_addr));
}

CANTransmitter::~CANTransmitter()
{
    closeSocket();
}

bool CANTransmitter::init()
{
    closeSocket();

    m_socketFd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (m_socketFd < 0)
    {
        perror("[CANTransmitter] Error creating SocketCAN socket");
        return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, m_interfaceName.c_str(), IFNAMSIZ - 1);

    if (ioctl(m_socketFd, SIOCGIFINDEX, &ifr) < 0)
    {
        perror(("[CANTransmitter] Error retrieving interface index for " + m_interfaceName).c_str());
        closeSocket();
        return false;
    }

    m_addr.can_family = AF_CAN;
    m_addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(m_socketFd, reinterpret_cast<struct sockaddr*>(&m_addr), sizeof(m_addr)) < 0)
    {
        perror(("[CANTransmitter] Error binding to " + m_interfaceName).c_str());
        closeSocket();
        return false;
    }

    return true;
}

bool CANTransmitter::sendFrame(uint32_t canId, const uint8_t* data, uint8_t dlc)
{
    if (m_socketFd < 0)
    {
        return false;
    }

    struct can_frame frame;
    std::memset(&frame, 0, sizeof(frame));
    frame.can_id = canId;
    frame.can_dlc = (dlc > CAN_STANDARD_DLC) ? CAN_STANDARD_DLC : dlc;

    if (data != nullptr && dlc > 0)
    {
        std::memcpy(frame.data, data, frame.can_dlc);
    }

    ssize_t bytesSent = write(m_socketFd, &frame, sizeof(struct can_frame));
    if (bytesSent != sizeof(struct can_frame))
    {
        perror("[CANTransmitter] Write error on CAN bus");
        return false;
    }

    return true;
}

void CANTransmitter::closeSocket()
{
    if (m_socketFd >= 0)
    {
        close(m_socketFd);
        m_socketFd = -1;
    }
}
