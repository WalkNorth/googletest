# Build spsc-benchmark example

add_executable(spsc-benchmark main.cc)

target_link_libraries(spsc-benchmark PRIVATE examples_lib)

# Enable threading
find_package(Threads REQUIRED)
target_link_libraries(spsc-benchmark PRIVATE Threads::Threads)

# Set C++ standard
target_compile_features(spsc-benchmark PRIVATE cxx_std_17)

# Optional: Add compiler flags for optimizations
if(NOT MSVC)
  target_compile_options(spsc-benchmark PRIVATE -Wall -Wextra -O2)
endif()
