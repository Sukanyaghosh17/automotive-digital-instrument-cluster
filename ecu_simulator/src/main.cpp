#include "VehicleECU.hpp"
#include <ncurses.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>

int main()
{
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);

    VehicleECU ecu("vcan0");
    if (!ecu.init())
    {
        endwin();
        std::cerr << "Failed to initialize Vehicle ECU on vcan0." << std::endl;
        std::cerr << "Ensure vcan0 is created (e.g. 'ip link add vcan0 type vcan && ip link set up vcan0')." << std::endl;
        return 1;
    }

    bool running = true;
    while (running)
    {
        int ch = getch();
        switch (ch)
        {
            case 'w':
            case 'W':
                ecu.throttle();
                break;
            case ' ':
                ecu.brake();
                break;
            case 'm':
            case 'M':
                ecu.cycleDriveMode();
                break;
            case 'p':
            case 'P':
                ecu.requestGear(Gear::P);
                break;
            case 'r':
            case 'R':
                ecu.requestGear(Gear::R);
                break;
            case 'n':
            case 'N':
                ecu.requestGear(Gear::N);
                break;
            case 'd':
            case 'D':
                ecu.requestGear(Gear::D);
                break;
            case 't':
            case 'T':
                ecu.triggerOverheatTest();
                break;
            case 'f':
            case 'F':
                ecu.triggerLowFuelTest();
                break;
            case 'a':
            case 'A':
                ecu.toggleAbsFault();
                break;
            case 'q':
            case 'Q':
                running = false;
                break;
            case ERR:
                ecu.idle();
                break;
            default:
                break;
        }

        ecu.step();

        // Render Clean Terminal ECU Dashboard
        const auto& pt = ecu.getPowertrain();
        const auto& st = ecu.getStatus();

        erase();
        attron(A_BOLD);
        mvprintw(1, 2, "==========================================================");
        mvprintw(2, 2, "     VEHICLE ECU SENSOR SIMULATOR (VIRTUAL ECU)           ");
        mvprintw(3, 2, "==========================================================");
        attroff(A_BOLD);

        mvprintw(5, 4, "VEHICLE DYNAMICS (CAN 0x100)");
        mvprintw(6, 4, "  Speed:      %3u km/h", pt.getSpeed());
        mvprintw(7, 4, "  Engine RPM: %4u RPM", pt.getRpm());
        mvprintw(8, 4, "  Gear:       [%s]", pt.getGearString().c_str());
        mvprintw(9, 4, "  Drive Mode: [%s]", pt.getDriveModeString().c_str());

        mvprintw(11, 4, "VEHICLE STATUS (CAN 0x200 & 0x300)");
        mvprintw(12, 4, "  Fuel Level: %3u%%  %s", st.getFuel(), st.isLowFuel() ? "[! LOW FUEL !]" : "");
        mvprintw(13, 4, "  Engine Temp: %3d C %s", st.getEngineTemp(), st.isOverheat() ? "[! OVERHEAT !]" : "");
        mvprintw(14, 4, "  ABS State:  %s", st.isAbsFault() ? "[! ABS FAULT !]" : "[OK]");

        attron(A_DIM);
        mvprintw(16, 2, "----------------------------------------------------------");
        mvprintw(17, 2, " CONTROLS:");
        mvprintw(18, 2, "   [W] Throttle     [SPACE] Brake        [M] Cycle Drive Mode");
        mvprintw(19, 2, "   [P/R/N/D] Shift  [T] Test Overheat    [F] Test Low Fuel");
        mvprintw(20, 2, "   [A] Toggle ABS   [Q] Quit");
        mvprintw(21, 2, "----------------------------------------------------------");
        attroff(A_DIM);

        refresh();
        usleep(50000); // 50ms tick rate (20 Hz)
    }

    endwin();
    return 0;
}
