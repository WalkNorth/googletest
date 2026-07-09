cmake_minimum_required(VERSION 3.10)
project(spdlog_demo LANGUAGES CXX)

# Allow overriding the location of a local spdlog checkout with -DSPDLOG_DIR=<path>
if(DEFINED SPDLOG_DIR AND EXISTS "${SPDLOG_DIR}/CMakeLists.txt")
  message(STATUS "Using spdlog from SPDLOG_DIR=${SPDLOG_DIR}")
  # When SPDLOG_DIR is outside this project, provide an out-of-tree binary dir
  get_filename_component(_spdlog_dir_abs "${SPDLOG_DIR}" ABSOLUTE)
  set(_spdlog_build_dir "${CMAKE_BINARY_DIR}/spdlog_external")
  add_subdirectory(${_spdlog_dir_abs} ${_spdlog_build_dir} EXCLUDE_FROM_ALL)
  set(SPDLOG_TARGET spdlog::spdlog)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/examples/lib/spdlog/CMakeLists.txt")
  message(STATUS "Using spdlog from ${CMAKE_SOURCE_DIR}/examples/lib/spdlog")
  set(_spdlog_build_dir "${CMAKE_BINARY_DIR}/examples_lib_spdlog")
  add_subdirectory("${CMAKE_SOURCE_DIR}/examples/lib/spdlog" ${_spdlog_build_dir} EXCLUDE_FROM_ALL)
  set(SPDLOG_TARGET spdlog::spdlog)
else()
  find_package(spdlog REQUIRED)
  set(SPDLOG_TARGET spdlog::spdlog)
endif()

add_executable(spdlog_demo main.cpp)
target_compile_features(spdlog_demo PRIVATE cxx_std_11)
target_link_libraries(spdlog_demo PRIVATE ${SPDLOG_TARGET})
