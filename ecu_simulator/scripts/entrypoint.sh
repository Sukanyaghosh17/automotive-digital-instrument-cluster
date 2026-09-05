#!/bin/sh
set -e

VCAN_IF="${VCAN_INTERFACE:-vcan0}"

# Ensure vcan interface exists
if ! ip link show "$VCAN_IF" > /dev/null 2>&1; then
    echo "[VehicleECU] Creating $VCAN_IF interface..."
    ip link add "$VCAN_IF" type vcan
    ip link set up "$VCAN_IF"
else
    echo "[VehicleECU] $VCAN_IF interface already exists."
fi

# Build if binary is missing
if [ ! -f "/app/vehicle_ecu" ]; then
    echo "[VehicleECU] Compiling binary..."
    make -C /app
fi

exec /app/vehicle_ecu
