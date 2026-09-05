# automotive-digital-instrument-cluster

A distributed automotive instrument cluster system simulating modern vehicle electronics.
Three-layer architecture: virtual vehicle ECU → CAN bus (`vcan0`) → translation gateway → Qt6 dashboard.

> **Inspired by** Ahsanbaloch's [Digital-Cockpit](https://github.com/Ahsanbaloch/Digital-Cockpit),
> extended with a multi-signal CAN matrix, explicit 3-container topology, drive mode profiles,
> and an integrated trip computer.

---

## 🔬 Verification Status

**Latest Update (Native Ubuntu / WSL Environment Validation):**
All three binaries (`vehicle_ecu`, `cluster_gateway`, and `cluster_ui`) have now been **successfully compiled from source** and launched locally to verify initialization and error-handling paths.

Full end-to-end CAN traffic propagation remains blocked on `vcan0` because the host PC currently has **Intel Virtualization (VT-x) disabled in BIOS/UEFI**, preventing WSL2 and Docker Desktop from loading the Linux kernel `vcan` module.

| Feature | Code Written | Compiled | Run / Observed |
| :--- | :---: | :---: | :---: |
| `vehicle_ecu` binary compilation | ✅ | ✅ | ✅ (Graceful exit code 1 on missing vcan0) |
| Speed simulation (0–260 km/h) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Speed → CAN `0x100` bytes 0–1 | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Speed → SOME/IP event `0x8001` | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Speed on Qt dashboard | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| RPM simulation (0–8000) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| RPM → CAN `0x100` bytes 2–3 | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| RPM on Qt dashboard | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Gear state machine (P/R/N/D) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Gear → CAN `0x100` byte 4 | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Gear on Qt dashboard | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Drive Mode (Eco/Comfort/Sport) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Drive Mode → CAN `0x100` byte 5 | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Drive Mode on Qt dashboard | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Fuel level simulation | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Fuel → CAN `0x200` byte 0 | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Fuel on Qt dashboard | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Engine temp simulation (-40 to +150°C) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Temp → CAN `0x200` bytes 1–2 | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Temp on Qt dashboard | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Warning flags bitmask (`0x300`) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Overheat warning (bit 0, >110°C) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Low Fuel warning (bit 1, <15%) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| ABS Fault warning (bit 2) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Warning lights on Qt dashboard | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Trip Computer (distance + avg speed) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| Keyboard controls (W/Space/M/P/R/N/D/T/F/A/Q) | ✅ | ✅ | ❌ (Awaiting vcan0 kernel module) |
| `cluster_gateway` vsomeip initialization | ✅ | ✅ | ✅ (Service 0x1234, events 0x8001-0x8003 registered) |
| `cluster_ui` QML engine loading | ✅ | ✅ | ✅ (Main.qml loaded cleanly via qrc:/ClusterUI/qml/; requires `qml6-module-qtqml-workerscript`) |
| `make build` (all 3 Docker images) | ✅ | ❌ | ❌ (Blocked: Docker requires BIOS VT-x enabled) |
| `make run` (all 3 containers live) | ✅ | ❌ | ❌ (Blocked: Docker requires BIOS VT-x enabled) |
| End-to-end signal flow (ECU → Gateway → UI) | ✅ | ✅ | ❌ (Blocked: Awaiting vcan0 kernel module) |

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    vehicle_ecu  (container 1)                   │
│   C++17 / SocketCAN — simulates powertrain & body state         │
│   Broadcasts CAN frames every 100 ms on vcan0                   │
│     0x100: Speed | RPM | Gear | Drive Mode                      │
│     0x200: Fuel Level | Coolant Temp                            │
│     0x300: Warning Flags bitmask                                │
└────────────────────────┬────────────────────────────────────────┘
                         │  SocketCAN frames (vcan0)
                         ▼
┌─────────────────────────────────────────────────────────────────┐
│                  cluster_gateway  (container 2)                 │
│   C++17 / vsomeip — CAN-to-SOME/IP translation                  │
│   Reads raw frames, decodes Big-Endian fields,                  │
│   republishes as typed SOME/IP events:                          │
│     0x8001: PowertrainEvent  (Speed, RPM, Gear, Mode)           │
│     0x8002: VehicleStatusEvent (Fuel, Temp)                     │
│     0x8003: WarningEvent (flags bitmask)                        │
└────────────────────────┬────────────────────────────────────────┘
                         │  SOME/IP / vsomeip IPC
                         ▼
┌─────────────────────────────────────────────────────────────────┐
│                   cluster_ui  (container 3)                     │
│   C++17 / Qt6 Quick / QML — instrument cluster dashboard        │
│   Avionics dark theme, real-time gauges, trip computer,         │
│   warning annunciator panel                                     │
└─────────────────────────────────────────────────────────────────┘
```

> **Networking:** All three containers use `network_mode: host`.
> This is required so they share the host's `vcan0` interface and
> vsomeip's multicast service-discovery sockets. It means the three
> containers are process groups sharing one network namespace with
> the host — not network-isolated services.

---

## Signal Specifications

### CAN Bus Matrix (11-bit IDs, 8-byte DLC, Big-Endian)

| CAN ID | Bytes | Signal | Type | Range |
| :--- | :--- | :--- | :--- | :--- |
| `0x100` | 0–1 | Speed | `uint16_t` | 0–260 km/h |
| `0x100` | 2–3 | Engine RPM | `uint16_t` | 0–8000 RPM |
| `0x100` | 4 | Gear | `uint8_t` | 0=P, 1=R, 2=N, 3=D |
| `0x100` | 5 | Drive Mode | `uint8_t` | 0=Eco, 1=Comfort, 2=Sport |
| `0x100` | 6–7 | Reserved | `uint8_t[2]` | `0x00` |
| `0x200` | 0 | Fuel Level | `uint8_t` | 0–100 % |
| `0x200` | 1–2 | Coolant Temp | `int16_t` | -40 to +150 °C |
| `0x200` | 3–7 | Reserved | `uint8_t[5]` | `0x00` |
| `0x300` | 0 | Warning Flags | `uint8_t` | bit0=Overheat, bit1=LowFuel, bit2=ABS |
| `0x300` | 1–7 | Reserved | `uint8_t[7]` | `0x00` |

### SOME/IP Service Matrix

| Event | Event ID | Payload | Byte Order |
| :--- | :--- | :--- | :--- |
| `PowertrainEvent` | `0x8001` | `[uint16 speed][uint16 rpm][uint8 gear][uint8 mode]` | Big-Endian |
| `VehicleStatusEvent` | `0x8002` | `[uint8 fuel][int16 temp]` | Big-Endian |
| `WarningEvent` | `0x8003` | `[uint8 flags]` | Bitfield |

Service ID `0x1234`, Instance `0x5678`, Eventgroup `0x0001`.

---

## Building and Running

### Prerequisites

- Linux or WSL2 (Ubuntu 22.04 recommended)
- Docker Desktop with WSL2 backend
- `vcan0` set up on the host **before** starting containers:

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

- X11 forwarding for the Qt GUI (WSLg handles this on Windows 11):

```bash
xhost +local:docker
```

### Build

```bash
make build
```

Builds all three Docker images. Expected time: 5–15 minutes on first build.

### Run

```bash
make run
```

Or in separate terminals for independent inspection:

```bash
make gateway   # Terminal 1
make gui       # Terminal 2
make ecu       # Terminal 3 — interactive keyboard controls
```

### ECU Keyboard Controls

> Defined in code — not yet tested.

| Key | Action |
| :--- | :--- |
| `W` | Throttle / Accelerate |
| `Space` | Brake / Decelerate |
| `M` | Cycle Drive Mode (Eco → Comfort → Sport) |
| `P` `R` `N` `D` | Gear shift request |
| `T` | Toggle Overheat test (90°C ↔ 118°C) |
| `F` | Toggle Low Fuel test (65% ↔ 10%) |
| `A` | Toggle ABS Fault |
| `Q` | Quit |

### Tear Down

```bash
make clean    # Stop containers
make fclean   # Stop + remove images and volumes
```

---

---

## Native Build & Verification Instructions (Without Docker)

If running directly on Ubuntu (or WSL with dependencies installed):

### 1. Build Tools & Libraries
```bash
sudo apt update && sudo apt install -y \
    build-essential cmake make git \
    libncurses-dev can-utils \
    libboost-all-dev \
    qt6-base-dev qt6-declarative-dev \
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts qml6-module-qtquick-templates \
    qml6-module-qtquick-window qml6-module-qtqml-workerscript
```

### 2. Build and Install vsomeip3 from Source
Since `vsomeip3` is not packaged in standard apt distributions:
```bash
git clone --depth 1 https://github.com/COVESA/vsomeip.git /tmp/vsomeip
mkdir -p /tmp/vsomeip/build && cd /tmp/vsomeip/build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DENABLE_SIGNAL_HANDLING=OFF ..
make -j$(nproc)
sudo make install
sudo ldconfig
```

### 3. Compile vehicle_ecu
```bash
cd ecu_simulator
make clean && make
# Output binary: ecu_simulator/vehicle_ecu
```

### 4. Compile cluster_gateway
```bash
cd cluster_gateway
cmake -B build -S .
cmake --build build
# Output binary: cluster_gateway/build/cluster_gateway
```

### 5. Compile cluster_ui
```bash
cd cluster_ui
cmake -B build -S .
cmake --build build
# Output binary: cluster_ui/build/cluster_ui
```

---

## Known Gaps & Hardware Requirements

- **Intel VT-x / AMD-V Required in BIOS:**
  On this Windows machine, `VirtualizationFirmwareEnabled` is currently `False` in the firmware. WSL2 and Docker Desktop cannot run without CPU hardware virtualization enabled in the BIOS/UEFI settings. Until enabled:
  - `vcan0` Linux kernel module cannot be loaded (WSL1 emulation lacks `AF_CAN`).
  - Docker Compose cannot build or start container images.
- **Bugs Caught and Fixed During Native Execution:**
  - `cluster_gateway/CMakeLists.txt`: `find_package(Boost REQUIRED COMPONENTS system thread log)` failed under modern Boost (1.87+/1.90) because `boost_system` is header-only and no longer provides a CMake component. Removed `system` from required components.
  - `cluster_gateway/src/ClusterGateway.cpp`: `offer_service()` was originally called before `offer_event()`, triggering vsomeip runtime error `rmc::register_provider_event: Registering events, after already offering the service is wrong behavior!`. Fixed the sequence to offer events prior to offering service.
  - `cluster_ui/src/main.cpp`: QML engine failed to load `qrc:/qml/Main.qml` because Qt6 `qt_add_qml_module` with URI `ClusterUI` generates resources at `qrc:/ClusterUI/qml/Main.qml`. Fixed the resource path in `main.cpp`.
