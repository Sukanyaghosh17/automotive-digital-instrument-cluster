#include "TripComputer.hpp"

TripComputer::TripComputer()
    : m_tripDistanceKm(0.0),
      m_averageSpeedKmh(0.0),
      m_speedAccumulator(0.0),
      m_samplesCount(0),
      m_lastUpdateTime(std::chrono::steady_clock::now())
{
}

void TripComputer::update(uint16_t currentSpeed)
{
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = now - m_lastUpdateTime;
    double dtHours = elapsed.count() / 3600.0;
    m_lastUpdateTime = now;

    // Prevent giant jumps on pauses or initial startup
    if (dtHours > 0.0 && dtHours < 0.05)
    {
        m_tripDistanceKm += currentSpeed * dtHours;
    }

    if (currentSpeed > 0)
    {
        m_speedAccumulator += currentSpeed;
        m_samplesCount++;
        m_averageSpeedKmh = m_speedAccumulator / m_samplesCount;
    }
}

void TripComputer::reset()
{
    m_tripDistanceKm = 0.0;
    m_averageSpeedKmh = 0.0;
    m_speedAccumulator = 0.0;
    m_samplesCount = 0;
    m_lastUpdateTime = std::chrono::steady_clock::now();
}
