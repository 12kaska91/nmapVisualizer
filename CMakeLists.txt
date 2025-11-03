cmake_minimum_required(VERSION 3.26)
project(nmapVisualizer C CXX)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBXMLPP REQUIRED libxml++-3.0)

add_executable(main)

target_sources(main
PRIVATE
    src/main.cpp
    src/graphics.hpp
    src/utils.hpp
)

# Link 
target_include_directories(main PRIVATE ${LIBXMLPP_INCLUDE_DIRS})
target_compile_options(main PRIVATE ${LIBXMLPP_CFLAGS_OTHER})
target_link_libraries(main PRIVATE SDL3::SDL3 SDL3_image::SDL3_image ${LIBXMLPP_LIBRARIES})

# Make sure the pkg-config variables are propagated for consumers
target_compile_definitions(main PRIVATE ${LIBXMLPP_CFLAGS})