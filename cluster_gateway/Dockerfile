FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies including Boost and CMake
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    iproute2 \
    can-utils \
    net-tools \
    && rm -rf /var/lib/apt/lists/*

# Pre-build and install vsomeip into system libraries during image build
RUN git clone --depth 1 https://github.com/COVESA/vsomeip.git /tmp/vsomeip && \
    mkdir -p /tmp/vsomeip/build && \
    cd /tmp/vsomeip/build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DENABLE_SIGNAL_HANDLING=OFF .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    rm -rf /tmp/vsomeip

WORKDIR /app
COPY . /app

RUN mkdir -p /etc/vsomeip && \
    cp /app/config/vsomeip_gateway.json /etc/vsomeip/vsomeip.json && \
    cmake -B build -S . && \
    cmake --build build -j$(nproc)

ENV VSOMEIP_CONFIGURATION=/etc/vsomeip/vsomeip.json

ENTRYPOINT ["/app/build/cluster_gateway"]
