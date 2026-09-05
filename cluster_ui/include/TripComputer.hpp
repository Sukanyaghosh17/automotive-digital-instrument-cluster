#ifndef TRIP_COMPUTER_HPP
#define TRIP_COMPUTER_HPP

#include <cstdint>
#include <chrono>

class TripComputer
{
public:
    TripComputer();

    void update(uint16_t currentSpeed);
    void reset();

    double getTripDistanceKm() const { return m_tripDistanceKm; }
    double getAverageSpeedKmh() const { return m_averageSpeedKmh; }

private:
    double m_tripDistanceKm;
    double m_averageSpeedKmh;
    double m_speedAccumulator;
    uint64_t m_samplesCount;
    std::chrono::steady_clock::time_point m_lastUpdateTime;
};

#endif // TRIP_COMPUTER_HPP
