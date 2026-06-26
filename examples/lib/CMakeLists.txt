# Library configuration for examples
# This is a header-only library that provides common utilities for examples

# Create a header-only library target
add_library(examples_lib INTERFACE)

# Add include directory
target_include_directories(examples_lib INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Set C++ standard requirement
target_compile_features(examples_lib INTERFACE cxx_std_17)
