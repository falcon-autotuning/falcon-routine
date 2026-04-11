# falcon-routine

Standard routine helpers for connecting to the instrument hub through the FFI interface and requesting measurements and data.

This module provides the C++ implementation layer for measurement routines that interact with the Falcon DSL instrument hub. It bridges user-written measurement libraries with the Falcon autotuner through the FFI boundary.

---

## Contents

| File | Purpose |
|------|---------|
| `include/falcon-routine/` | Public headers for routine helpers and instrument hub interface |
| `src/` | Implementation of routine handlers and FFI binding utilities |

---

## Quick Start

### 1. Build & Install

```bash
# From falcon-lib root
make routine           # shortcut if defined in the root Makefile, OR:

cd routine
make install          # builds release + installs to /opt/falcon
```

### 2. Use in another project

```CMake
find_package(falcon-routine CONFIG REQUIRED
  PATHS /opt/falcon/lib/cmake/falcon-routine
)

target_link_libraries(my-target PRIVATE falcon::falcon-routine)
```

```C++

#include "falcon-routine/RoutineHelpers.hpp"

// Connect to instrument hub and request measurements
auto hub = falcon::routine::connect_to_hub();
auto measurement = hub.request_measurement(params);
```

## Building from Source

```bash
cd routine
mkdir -p build/release && cd build/release
cmake ../.. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=../../.vcpkg/scripts/buildsystems/vcpkg.cmake \
  -Dfalcon_typing_DIR=/opt/falcon/lib/cmake/falcon-typing \
  -G Ninja
ninja
sudo cmake --install . --prefix /opt/falcon
```

## Running Tests

```bash
cd routine
make test              # release
make test-debug        # debug
make test-verbose      # verbose output
```

## Uninstalling

```bash
cd routine
make uninstall
```

## Dependencies

- CMake >= 3.20
- C++17 compiler
- nlohmann-json
- spdlog
- cnats
- falcon-database
- falcon-comms
- falcon-core
- Google Test (for tests)

## License

MPL-2.0
