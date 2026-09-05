FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    build-essential \
    iproute2 \
    can-utils \
    libncurses5-dev \
    libncursesw5-dev \
    ncurses-term \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

RUN chmod +x /app/scripts/entrypoint.sh && \
    make -C /app

ENV TERM=xterm-256color

ENTRYPOINT ["/app/scripts/entrypoint.sh"]
