# Build spsc-producer-consumer example

add_executable(spsc-producer-consumer main.cc)

target_link_libraries(spsc-producer-consumer PRIVATE examples_lib)

# Enable threading
find_package(Threads REQUIRED)
target_link_libraries(spsc-producer-consumer PRIVATE Threads::Threads)

# Set C++ standard
target_compile_features(spsc-producer-consumer PRIVATE cxx_std_17)

# Optional: Add compiler flags for optimizations
if(NOT MSVC)
  target_compile_options(spsc-producer-consumer PRIVATE -Wall -Wextra)
endif()
