# Examples

This directory contains demonstration programs that showcase how to use googletest and its utilities.

## Building Examples

By default, examples are not built. To build them, use the `BUILD_EXAMPLES` option:

```bash
# Create build directory
mkdir build && cd build

# Configure CMake with examples enabled
cmake -DBUILD_EXAMPLES=ON ..

# Build
cmake --build .
```

## Available Examples

### SPSC Queue Examples

The SPSC (Single-Producer, Single-Consumer) queue is a lock-free data structure for efficient inter-thread communication.

#### spsc-producer-consumer

Basic demonstration of the SPSC queue with producer and consumer threads.

**Features:**
- Basic push and pop operations
- Queue capacity and full behavior
- Multi-threaded producer-consumer pattern
- Support for different data types

**Run:**
```bash
./bin/spsc-producer-consumer
```

**Example Output:**
```
=== SPSC Queue Producer-Consumer Demo ===

Example 1: Basic Push and Pop (Single Thread)
  Pushing: 10, 20, 30
  Queue size: 3
  Popping: 10 20 30
  Queue empty: yes
  ...
```

#### spsc-benchmark

Performance benchmarking of the SPSC queue with various message counts and configurations.

**Features:**
- Configurable message counts (100 to 100,000)
- Throughput measurement (messages/second)
- Latency statistics (min, average, max)
- Multiple iterations for statistical significance

**Run:**
```bash
./bin/spsc-benchmark
```

**Example Output:**
```
=== SPSC Queue Performance Benchmark ===

Running warmup iterations...
Warmup complete.

Test Name                      Messages   Queue Cap   Avg Latency  Throughput
...
Small (100 msg)                       100            64      123.45 us   0.81 M/sec
Medium (1k msg)                      1000            64      115.67 us   8.64 M/sec
...
```

## Directory Structure

```
examples/
├── lib/                      # Shared library code
│   ├── include/
│   │   └── spsc_queue.h      # SPSC queue implementation
│   └── CMakeLists.txt
├── demos/                    # Demo applications
│   ├── spsc-producer-consumer/
│   │   ├── main.cc
│   │   └── CMakeLists.txt
│   ├── spsc-benchmark/
│   │   ├── main.cc
│   │   └── CMakeLists.txt
│   └── CMakeLists.txt
├── CMakeLists.txt
└── README.md
```

## Adding New Examples

To add a new example:

1. Create a new directory under `demos/`:
   ```bash
   mkdir examples/demos/my-demo
   ```

2. Create `main.cc` with your demo code

3. Create `CMakeLists.txt`:
   ```cmake
   add_executable(my-demo main.cc)
   target_link_libraries(my-demo PRIVATE examples_lib)
   find_package(Threads REQUIRED)
   target_link_libraries(my-demo PRIVATE Threads::Threads)
   target_compile_features(my-demo PRIVATE cxx_std_17)
   ```

4. Add the subdirectory to `demos/CMakeLists.txt`:
   ```cmake
   add_subdirectory(my-demo)
   ```

## Requirements

- C++17 or later
- CMake 3.13+
- Threading support

## Dependencies

Examples use:
- **Google Test**: For utilities (header-only)
- **Standard C++ Library**: For threading, containers, and synchronization

Examples do NOT depend on the Google Test framework itself (no `#include <gtest/gtest.h>` in demo code).
