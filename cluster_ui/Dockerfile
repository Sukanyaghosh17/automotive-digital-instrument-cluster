FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install Qt6, build essentials, and X11 rendering dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    qt6-base-dev \
    qt6-declarative-dev \
    qml6-module-qtquick \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    qml6-module-qtquick-templates \
    qml6-module-qtqml-workerscript \
    libgl1-mesa-dev \
    libx11-dev \
    libxkbcommon-x11-0 \
    libxcb1 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libxcb-xinerama0 \
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
    cp /app/config/vsomeip_client.json /etc/vsomeip/vsomeip.json && \
    cmake -B build -S . && \
    cmake --build build -j$(nproc)

ENV VSOMEIP_CONFIGURATION=/etc/vsomeip/vsomeip.json
ENV QT_X11_NO_MITSHM=1

ENTRYPOINT ["/app/build/cluster_ui"]
