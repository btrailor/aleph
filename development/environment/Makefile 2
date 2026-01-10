# Monome Aleph Development Makefile
# Convenience commands for common development tasks

.PHONY: help setup dev build-bees build-lines shell clean-docker

help:
	@echo "Monome Aleph Development Commands"
	@echo "=================================="
	@echo ""
	@echo "Setup:"
	@echo "  make setup        - Build Docker image and set up development environment"
	@echo ""
	@echo "Development:"
	@echo "  make dev          - Start interactive development shell in Docker"
	@echo "  make shell        - Alias for 'make dev'"
	@echo ""
	@echo "Building (requires toolchains to be built first):"
	@echo "  make build-bees   - Build BEES controller app (in Docker)"
	@echo "  make build-lines  - Build lines DSP module (in Docker)"
	@echo ""
	@echo "Maintenance:"
	@echo "  make clean-docker - Remove Docker containers and images"
	@echo ""
	@echo "Quick Start:"
	@echo "  1. make setup     - First time setup"
	@echo "  2. make dev       - Start development environment"
	@echo "  3. Inside container: /aleph/build-toolchains.sh"
	@echo "  4. Inside container: cd /aleph/apps/bees && make"
	@echo ""
	@echo "For more information, see QUICKSTART.md or DEVELOPMENT_SETUP.md"

setup:
	@echo "Setting up Monome Aleph development environment..."
	@./setup-dev.sh

dev:
	@echo "Starting development environment..."
	@docker run --rm -it -v "$$(pwd)/aleph:/aleph" -v "$$(pwd)/toolchain:/toolchain" aleph-dev

shell: dev

build-bees:
	@echo "Building BEES controller app..."
	@docker run --rm -v "$$(pwd)/aleph:/aleph" -v "$$(pwd)/toolchain:/toolchain" aleph-dev bash -c "cd /aleph/apps/bees && make"

build-lines:
	@echo "Building lines DSP module..."
	@docker run --rm -v "$$(pwd)/aleph:/aleph" -v "$$(pwd)/toolchain:/toolchain" aleph-dev bash -c "cd /aleph/modules/lines && make"

clean-docker:
	@echo "Cleaning up Docker containers and images..."
	@docker rmi aleph-dev 2>/dev/null || true
	@echo "Docker cleanup complete"
