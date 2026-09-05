# ==============================================================================
# automotive-digital-instrument-cluster — Master Makefile
# ==============================================================================

DOCKER_COMPOSE ?= docker compose

all: build

build:
	$(DOCKER_COMPOSE) build

# Run entire stack: Starts gateway and GUI in background, attaches ECU interactively
run:
	xhost +local:docker 2>/dev/null || true
	$(DOCKER_COMPOSE) up -d cluster_gateway cluster_gui
	$(DOCKER_COMPOSE) run --rm -it vehicle_ecu

# Individual service targets for multi-terminal workflows
ecu:
	$(DOCKER_COMPOSE) run --rm -it vehicle_ecu

gateway:
	$(DOCKER_COMPOSE) up --build cluster_gateway

gui:
	xhost +local:docker 2>/dev/null || true
	$(DOCKER_COMPOSE) up --build cluster_gui

# Cleanup targets
clean:
	$(DOCKER_COMPOSE) down

fclean: clean
	$(DOCKER_COMPOSE) down -v --rmi all --remove-orphans

.PHONY: all build run ecu gateway gui clean fclean
