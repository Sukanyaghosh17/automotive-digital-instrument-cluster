# automotive-digital-instrument-cluster

A distributed automotive instrument cluster system simulating modern vehicle electronics.
Three-layer architecture: virtual vehicle ECU → CAN bus (`vcan0`) → translation gateway → Qt6 dashboard.

> **Inspired by** Ahsanbaloch's [Digital-Cockpit](https://github.com/Ahsanbaloch/Digital-Cockpit),
> extended with a multi-signal CAN matrix, explicit 3-container topology, drive mode profiles,
> and an integrated trip computer.

---

## ⚠️ Verification Status

**Nothing in this project has been compiled or run on any machine yet.**

All code was written and reviewed statically. The table below tracks every claimed feature
against its actual status. Items will be updated to ✅ only when real terminal output confirms them.

| Feature | Code Written | Compiled | Run / Observed |
| :--- | :---: | :---: | :---: |
| Speed simulation (0–260 km/h) | ✅ | ❌ | ❌ |
| Speed → CAN `0x100` bytes 0–1 | ✅ | ❌ | ❌ |
| Speed → SOME/IP event `0x8001` | ✅ | ❌ | ❌ |
| Speed on Qt dashboard | ✅ | ❌ | ❌ |
| RPM simulation (0–8000) | ✅ | ❌ | ❌ |
| RPM → CAN `0x100` bytes 2–3 | ✅ | ❌ | ❌ |
| RPM on Qt dashboard | ✅ | ❌ | ❌ |
| Gear state machine (P/R/N/D) | ✅ | ❌ | ❌ |
| Gear → CAN `0x100` byte 4 | ✅ | ❌ | ❌ |
| Gear on Qt dashboard | ✅ | ❌ | ❌ |
| Drive Mode (Eco/Comfort/Sport) | ✅ | ❌ | ❌ |
| Drive Mode → CAN `0x100` byte 5 | ✅ | ❌ | ❌ |
| Drive Mode on Qt dashboard | ✅ | ❌ | ❌ |
| Fuel level simulation | ✅ | ❌ | ❌ |
| Fuel → CAN `0x200` byte 0 | ✅ | ❌ | ❌ |
| Fuel on Qt dashboard | ✅ | ❌ | ❌ |
| Engine temp simulation (-40 to +150°C) | ✅ | ❌ | ❌ |
| Temp → CAN `0x200` bytes 1–2 | ✅ | ❌ | ❌ |
| Temp on Qt dashboard | ✅ | ❌ | ❌ |
| Warning flags bitmask (`0x300`) | ✅ | ❌ | ❌ |
| Overheat warning (bit 0, >110°C) | ✅ | ❌ | ❌ |
| Low Fuel warning (bit 1, <15%) | ✅ | ❌ | ❌ |
| ABS Fault warning (bit 2) | ✅ | ❌ | ❌ |
| Warning lights on Qt dashboard | ✅ | ❌ | ❌ |
| Trip Computer (distance + avg speed) | ✅ | ❌ | ❌ |
| Keyboard controls (W/Space/M/P/R/N/D/T/F/A/Q) | ✅ | ❌ | ❌ |
| `make build` (all 3 Docker images) | ✅ | ❌ | ❌ |
| `make run` (all 3 containers live) | ✅ | ❌ | ❌ |
| End-to-end signal flow (ECU → Gateway → UI) | ✅ | ❌ | ❌ |

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

## Known Gaps / Not Yet Validated

- **No runtime confirmation exists for any signal path.** The entire
  CAN → Gateway → UI pipeline has been code-reviewed but never executed.
  Endianness alignment, SOME/IP service discovery, X11 forwarding, and
  keyboard handling all need real execution to validate.
- **WSL2 kernel may not include the `vcan` module** out of the box.
  If `modprobe vcan` fails, a custom WSL2 kernel build is required.
- **Qt6 + OpenGL inside Docker on WSL2** is an untested surface area.
  A software-rendering fallback (`QT_QUICK_BACKEND=software`) may be needed.

---

## Architecture Overview

The system strictly decouples the powertrain dynamics, network translation, and presentation into three standalone layers:

```
┌────────────────────────────────────────────────────────────────────────┐
│                        Vehicle ECU Simulator                           │
│   (Virtual Powertrain & Body ECU — C++17 / SocketCAN)                  │
│                                                                        │
│   - Multi-mode dynamics (Eco, Comfort, Sport)                          │
│   - Realistic throttle, braking, and gear state machine (P/R/N/D)      │
│   - Engine thermal warmup & fuel depletion simulation                  │
│   - Warning flag generation (Engine Overheat, Low Fuel, ABS Fault)     │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
                                    │ SocketCAN Frames (vcan0)
                                    │ [0x100, 0x200, 0x300]
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                       Cluster Gateway Service                          │
│   (CAN-to-SOME/IP Translation Gateway — C++17 / vsomeip)               │
│                                                                        │
│   - SocketCAN RAW listener on vcan0                                    │
│   - Frame decoding & Big-Endian deserialization                        │
│   - Publishes typed SOME/IP Events over Service 0x1234, Instance 0x5678│
│       • 0x8001: PowertrainEvent      (Speed, RPM, Gear, Drive Mode)    │
│       • 0x8002: VehicleStatusEvent   (Fuel Level, Coolant Temp)        │
│       • 0x8003: WarningEvent         (Annunciator Warning Flags)       │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
                                    │ SOME/IP Protocol (IPC / Multicast SD)
                                    │ Eventgroup 0x0001
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                       Instrument Cluster UI                            │
│   (Digital Cockpit Dashboard — C++17 / Qt6 Quick / QML)                │
│                                                                        │
│   - High-contrast Avionics Cockpit / Cyber-Instrument dark theme       │
│   - Real-time speedometer & progressive tachometer (0-8000 RPM)        │
│   - Integrated Trip Computer (Trip Distance & Running Average Speed)   │
│   - Dynamic Annunciator warning lights (Engine, ABS, Low Fuel)         │
└────────────────────────────────────────────────────────────────────────┘
```

> **Note on Container Networking:** All three containers are configured with `network_mode: host`. This design choice is required because Linux virtual CAN interfaces (`vcan0`) and vsomeip daemon discovery/IPC sockets bind directly to the kernel network namespace. Using `network_mode: host` allows the virtual ECU and Gateway to share `vcan0` with zero packet-forwarding overhead, while enabling immediate multicast service discovery and local X11 GUI forwarding.

---

## Network & Signal Specifications

### 1. CAN Bus Matrix (Standard 11-bit IDs, 8-byte DLC, Big-Endian)

| CAN ID | Frame Name | Bytes | Signal | Type | Resolution / Range | Description |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **`0x100`** | `Powertrain_Frame` | 0–1 | Speed | `uint16_t` | 1 km/h (0 – 260) | Vehicle speed |
| | | 2–3 | Engine RPM | `uint16_t` | 1 RPM (0 – 8000) | Tachometer engine speed |
| | | 4 | Gear | `uint8_t` | Enum (0–3) | `0=P, 1=R, 2=N, 3=D` |
| | | 5 | Drive Mode | `uint8_t` | Enum (0–2) | `0=Eco, 1=Comfort, 2=Sport` |
| | | 6–7 | Reserved | `uint8_t[2]`| `0x00` | Padding |
| **`0x200`** | `VehicleStatus_Frame` | 0 | Fuel Level | `uint8_t` | 1% (0 – 100) | Remaining fuel tank capacity |
| | | 1–2 | Coolant Temp | `int16_t` | 1°C (-40 to +150) | Engine coolant temperature |
| | | 3–7 | Reserved | `uint8_t[5]`| `0x00` | Padding |
| **`0x300`** | `Warning_Frame` | 0 | Flags Bitmask | `uint8_t` | Bitfield | Bit 0: Overheat (`temp > 110°C`)<br>Bit 1: Low Fuel (`fuel < 15%`)<br>Bit 2: ABS Fault (`0=OK, 1=Fault`) |
| | | 1–7 | Reserved | `uint8_t[7]`| `0x00` | Padding |

### 2. SOME/IP Service Matrix

- **Service ID**: `0x1234` (4660)
- **Instance ID**: `0x5678` (22136)
- **Eventgroup ID**: `0x0001` (1)

| Event Name | Event ID | Payload Structure | Payload Size | Serialization |
| :--- | :--- | :--- | :--- | :--- |
| **`PowertrainEvent`** | `0x8001` (32769) | `[uint16 speed][uint16 rpm][uint8 gear][uint8 mode]` | 6 Bytes | Big-Endian (Network Order) |
| **`VehicleStatusEvent`** | `0x8002` (32770) | `[uint8 fuel][int16 temp]` | 3 Bytes | Big-Endian (Network Order) |
| **`WarningEvent`** | `0x8003` (32771) | `[uint8 warning_flags]` | 1 Byte | Bitfield |

---

## Enhancements Over the Reference Project

1. **True 3-Container Topology**: The original reference project packaged the SOME/IP daemon and Qt GUI inside a single container via a background shell script (`CANService & appInstrument_cluster`). This project separates them into distinct `cluster_gateway` and `cluster_gui` services with isolated builds and lifecycles.
2. **Pre-Built Dependency Caching**: Rather than cloning and compiling `vsomeip` at container runtime on every single startup (which caused 3–5 minute cold-start delays), dependencies are compiled during `docker build`. Containers start in under a second.
3. **Decoded Signal Architecture**: Rather than tunneling raw CAN bytes through an opaque SOME/IP event, the Gateway actively parses CAN frames into strongly-typed telemetry events.
4. **Dead-Weight Elimination**: Stripped out unused Franca IDL / CommonAPI generator files (`.fidl`, `.fdepl`, `src-gen/`) and unneeded CommonAPI runtimes that were left behind in the reference repo.
5. **Original Features**:
   - **Drive Mode Profiles (Eco / Comfort / Sport)**: Modifies throttle response curves, shift thresholds, and fuel burn rates.
   - **Integrated Trip Computer**: Calculates live odometer trip distance ($km$) and running average speed ($km/h$).
   - **Avionics Cockpit Visual Identity**: Modern dark obsidian instrument UI with progressive tachometer bar and warning annunciator.

---

## Building and Running

### Prerequisites
On Linux or WSL2 with an X11 server (e.g. VcXsrv, WSLg):
```bash
xhost +local:docker
```

### Quick Start (Master Makefile)
Build and run the entire stack:
```bash
# Build all container images
make build

# Launch Gateway and GUI in background, attach interactive ECU terminal:
make run
```

### Multi-Terminal Workflow
For independent inspection and debugging, start each service in its own terminal:

1. **Terminal 1 — Cluster Gateway Service**:
   ```bash
   make gateway
   ```
2. **Terminal 2 — Instrument Cluster GUI**:
   ```bash
   make gui
   ```
3. **Terminal 3 — Vehicle ECU Simulator**:
   ```bash
   make ecu
   ```

### ECU Controls
In the interactive ECU terminal window:
- <kbd>W</kbd>: Throttle / Accelerate
- <kbd>Space</kbd>: Brake / Decelerate
- <kbd>M</kbd>: Cycle Drive Mode (`Eco` $\to$ `Comfort` $\to$ `Sport`)
- <kbd>P</kbd>, <kbd>R</kbd>, <kbd>N</kbd>, <kbd>D</kbd>: Request Gear Shift (only valid at standstill for P/N/R)
- <kbd>T</kbd>: Trigger Overheat Test (toggles 90°C $\leftrightarrow$ 118°C)
- <kbd>F</kbd>: Trigger Low Fuel Test (toggles 65% $\leftrightarrow$ 10%)
- <kbd>A</kbd>: Toggle ABS Fault state
- <kbd>Q</kbd>: Quit

### Tear Down
```bash
make clean    # Stops containers
make fclean   # Removes containers, volumes, and built images
```
