cmake_minimum_required(VERSION 3.26)
project(hello C CXX)

find_package(SDL3 REQUIRED)

add_executable(main)

target_sources(main
PRIVATE
    src/main.cpp
    src/graphics.hpp
    src/utils.hpp
)

target_link_libraries(main SDL3::SDL3)