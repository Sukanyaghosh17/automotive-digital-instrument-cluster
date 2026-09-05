#include "ClusterGateway.hpp"
#include <iostream>
#include <csignal>

static ClusterGateway* g_gateway = nullptr;

void handleSignal(int sig)
{
    std::cout << "\n[ClusterGateway] Received termination signal (" << sig << "), shutting down..." << std::endl;
    if (g_gateway)
    {
        g_gateway->stop();
    }
    exit(0);
}

int main()
{
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);

    try
    {
        std::cout << "[ClusterGateway] Starting Automotive Digital Instrument Cluster Gateway..." << std::endl;
        ClusterGateway gateway("vcan0");
        g_gateway = &gateway;

        gateway.init();
        gateway.start();
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ClusterGateway] Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
