.PHONY: help configure build test clean install vcpkg-bootstrap docker-up docker-down

# Build preset (user can override: make build PRESET=linux-gcc-release)
PRESET ?= linux-clang-release
CMAKE_BUILD_DIR := build/$(PRESET)

help:
	@echo "Falcon Routine Build System"
	@echo "============================="
	@echo ""
	@echo "Available presets:"
	@cmake --list-presets=all
	@echo ""
	@echo "Usage:"
	@echo "  make configure PRESET=<preset>  - Configure build (default: $(PRESET))"
	@echo "  make build PRESET=<preset>      - Build (default: $(PRESET))"
	@echo "  make test PRESET=<preset>       - Run tests with Docker (default: $(PRESET))"
	@echo "  make install PRESET=<preset>    - Install to system"
	@echo "  make clean                      - Clean all build artifacts"
	@echo ""
	@echo "Docker targets:"
	@echo "  make docker-up                  - Start PostgreSQL test database"
	@echo "  make docker-down                - Stop PostgreSQL test database"
	@echo ""
	@echo "Examples:"
	@echo "  make build                                      # Build with clang (default)"
	@echo "  make build PRESET=linux-gcc-release             # Build with gcc"
	@echo "  make test PRESET=linux-clang-release            # Run tests"
	@echo "  make install PRESET=linux-clang-release         # Install"

vcpkg-bootstrap:
	@echo "Bootstrapping vcpkg..."
	MAKELEVEL=0 cmake -P cmake/bootstrap/bootstrap-vcpkg.cmake

configure: vcpkg-bootstrap
	@echo "Configuring $(PRESET)..."
	cmake --preset $(PRESET)

build: configure
	@echo "Building $(PRESET)..."
	cmake --build --preset $(PRESET)

docker-up: 
	@echo "Starting test services (PostgreSQL + NATS)..."
	@docker-compose -f docker-compose.test.yml up -d
	@echo "Waiting for services to be ready..."
	@for i in 1 2 3 4 5; do \
		if docker-compose -f docker-compose.test.yml exec -T postgres pg_isready -U falcon_test >/dev/null 2>&1 && \
		   docker-compose -f docker-compose.test.yml exec -T nats wget -q -O- http://localhost:8222/healthz >/dev/null 2>&1; then \
			echo "✓ Services are ready"; \
			exit 0; \
		fi; \
		echo "  Waiting... ($$i/5)"; \
		sleep 3; \
	done; \
	echo "✗ Services failed to start"; \
	exit 1

docker-down:
	@echo "Stopping PostgreSQL test database..."
	@docker-compose -f docker-compose.test.yml down
	@echo "✓ PostgreSQL stopped"

test: docker-up build
	@echo "Running tests for $(PRESET)..."
	@TEST_DATABASE_URL="$(TEST_DATABASE_URL)" \
	 TEST_NATS_URL="$(TEST_NATS_URL)" \
	 LOG_FILE="$(LOG_FILE)" \
	 LOG_LEVEL="$(LOG_LEVEL)" \
	 ctest --preset $(PRESET) --output-on-failure; \
	EXIT_CODE=$$?; \
	$(MAKE) docker-down; \
	exit $$EXIT_CODE

install: build
	@echo "Installing $(PRESET) to system..."
	cmake --install $(CMAKE_BUILD_DIR)

clean:
	@echo "Cleaning all build artifacts..."
	rm -rf build vcpkg_installed
	@echo "✓ Clean complete"
