#include "PowertrainModel.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <cmath>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[TEST FAILED] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            return 1; \
        } \
    } while (0)

int main()
{
    std::cout << "============================================================" << std::endl;
    std::cout << "        RUNNING POWERTRAIN MODEL NUMERIC RPM TESTS         " << std::endl;
    std::cout << "============================================================" << std::endl;

    PowertrainModel model;

    // Test 1: Full 0-260 km/h sweep in all DriveModes
    struct ModeConfig {
        DriveMode mode;
        const char* name;
    };

    std::vector<ModeConfig> modes = {
        {DriveMode::Comfort, "COMFORT"},
        {DriveMode::Eco,     "ECO"},
        {DriveMode::Sport,   "SPORT"}
    };

    struct GearRange {
        uint16_t minSpeed;
        uint16_t maxSpeed;
        int gearNumber;
    };

    std::vector<GearRange> gearRanges = {
        {0,   24,  1},
        {25,  49,  2},
        {50,  79,  3},
        {80,  119, 4},
        {120, 159, 5},
        {160, 260, 6}
    };

    std::vector<uint16_t> shiftPoints = {25, 50, 80, 120, 160};

    for (const auto& mc : modes)
    {
        std::cout << "\n>>> Testing Drive Mode: " << mc.name << " <<<" << std::endl;
        model.setDriveMode(mc.mode);
        model.setGear(Gear::D);

        // Record RPMs across 0..260 km/h sweep
        std::vector<uint16_t> rpms(261, 0);
        for (uint16_t s = 0; s <= 260; ++s)
        {
            model.setSpeed(s);
            rpms[s] = model.getRpm();
        }

        // Check standstill idle RPM
        TEST_ASSERT(rpms[0] == 800, "Standstill (speed 0) RPM must equal idle (800)");

        // Check monotonicity within each gear band
        for (const auto& gr : gearRanges)
        {
            for (uint16_t s = gr.minSpeed; s < gr.maxSpeed; ++s)
            {
                TEST_ASSERT(rpms[s + 1] >= rpms[s],
                            "RPM decreased within Gear " + std::to_string(gr.gearNumber) +
                            " at speed " + std::to_string(s) + " -> " + std::to_string(s + 1) +
                            " (" + std::to_string(rpms[s]) + " -> " + std::to_string(rpms[s + 1]) + ")");
            }
            std::cout << "  [PASS] Gear " << gr.gearNumber << " ("
                      << std::setw(3) << gr.minSpeed << "-" << std::setw(3) << gr.maxSpeed
                      << " km/h): Monotonically non-decreasing ("
                      << rpms[gr.minSpeed] << " -> " << rpms[gr.maxSpeed] << " RPM)" << std::endl;
        }

        // Check upshift boundaries
        std::cout << "  Upshift boundaries check:" << std::endl;
        for (uint16_t sp : shiftPoints)
        {
            uint16_t rpmBefore = rpms[sp - 1];
            uint16_t rpmAfter  = rpms[sp];

            TEST_ASSERT(rpmAfter < rpmBefore,
                        "Upshift at " + std::to_string(sp) + " km/h failed to drop RPM: " +
                        std::to_string(rpmBefore) + " -> " + std::to_string(rpmAfter));

            double dropPct = static_cast<double>(rpmBefore - rpmAfter) / static_cast<double>(rpmBefore) * 100.0;

            TEST_ASSERT(dropPct <= 40.0,
                        "Upshift drop at " + std::to_string(sp) + " km/h exceeded 40%: " +
                        std::to_string(dropPct) + "% (" + std::to_string(rpmBefore) + " -> " + std::to_string(rpmAfter) + ")");

            TEST_ASSERT(dropPct >= 15.0,
                        "Upshift drop at " + std::to_string(sp) + " km/h too low: " +
                        std::to_string(dropPct) + "% (" + std::to_string(rpmBefore) + " -> " + std::to_string(rpmAfter) + ")");

            std::cout << "    Shift at " << std::setw(3) << sp << " km/h: "
                      << std::setw(4) << rpmBefore << " RPM -> "
                      << std::setw(4) << rpmAfter << " RPM (Drop: "
                      << std::fixed << std::setprecision(1) << dropPct << "%) [PASS]" << std::endl;
        }

        // Check top speed RPM <= redline
        TEST_ASSERT(rpms[260] <= 8000, "Top speed RPM exceeds 8000 RPM redline");
        std::cout << "  [PASS] Top speed (260 km/h) RPM: " << rpms[260] << " RPM (<= 8000 RPM)" << std::endl;
    }

    // Test 2: Reverse gear sweep [0..35 km/h]
    std::cout << "\n>>> Testing Reverse Gear (R) <<<" << std::endl;
    model.setSpeed(0);
    model.setGear(Gear::R);
    model.setDriveMode(DriveMode::Comfort);

    uint16_t prevRpm = model.getRpm();
    TEST_ASSERT(prevRpm == 800, "Reverse standstill RPM must equal 800");

    for (uint16_t s = 1; s <= 35; ++s)
    {
        model.setSpeed(s);
        uint16_t currentRpm = model.getRpm();
        TEST_ASSERT(currentRpm >= prevRpm, "Reverse RPM decreased at speed " + std::to_string(s));
        prevRpm = currentRpm;
    }
    std::cout << "  [PASS] Reverse gear (0-35 km/h) monotonically non-decreasing (800 -> "
              << prevRpm << " RPM)" << std::endl;

    // Test 3: Park and Neutral idle check
    std::cout << "\n>>> Testing Park (P) and Neutral (N) <<<" << std::endl;
    model.setSpeed(0);
    model.setGear(Gear::P);
    TEST_ASSERT(model.getRpm() == 800, "Gear P RPM must be 800");

    model.setGear(Gear::N);
    TEST_ASSERT(model.getRpm() == 800, "Gear N RPM must be 800");
    std::cout << "  [PASS] Gear P and N hold RPM at 800 (idle)" << std::endl;

    std::cout << "\n============================================================" << std::endl;
    std::cout << "             ALL NUMERIC TESTS PASSED CLEANLY!              " << std::endl;
    std::cout << "============================================================" << std::endl;

    return 0;
}
