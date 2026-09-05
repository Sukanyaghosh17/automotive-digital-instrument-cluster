#ifndef CAN_FRAME_DECODER_HPP
#define CAN_FRAME_DECODER_HPP

#include <linux/can.h>
#include <cstdint>

struct PowertrainData
{
    uint16_t speed;      // km/h (0 - 260)
    uint16_t rpm;        // RPM  (0 - 8000)
    uint8_t  gear;       // 0=P, 1=R, 2=N, 3=D
    uint8_t  driveMode;  // 0=Eco, 1=Comfort, 2=Sport
};

struct VehicleStatusData
{
    uint8_t fuelLevel;   // 0 - 100%
    int16_t engineTemp;  // -40 to +150 °C
};

struct WarningData
{
    uint8_t warningFlags; // Bit 0: Overheat, Bit 1: Low Fuel, Bit 2: ABS
};

class CANFrameDecoder
{
public:
    static bool decodePowertrain(const struct can_frame& frame, PowertrainData& outData);
    static bool decodeVehicleStatus(const struct can_frame& frame, VehicleStatusData& outData);
    static bool decodeWarnings(const struct can_frame& frame, WarningData& outData);
};

#endif // CAN_FRAME_DECODER_HPP
