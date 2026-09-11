cmake_minimum_required(VERSION 3.10)
project(free_demo LANGUAGES CXX)

add_executable(free_demo main.cpp)
target_compile_features(free_demo PRIVATE cxx_std_11)
