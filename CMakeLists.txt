cmake_minimum_required(VERSION 3.26)
project(nmapVisualizer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)

# libxml++
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

# gtkmm
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)


# Executable
add_executable(main
    src/main.cpp
    src/graphics.hpp
    src/utils.hpp
)

# Include directories
target_include_directories(main PRIVATE ${LIBXML2_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS})

# Compiler flags from pkg-config
target_compile_options(main PRIVATE ${LIBXML2_CFLAGS_OTHER} ${GTKMM_CFLAGS_OTHER})

# Link libraries
target_link_libraries(main PRIVATE ${LIBXML2_LIBRARIES} ${GTKMM_LIBRARIES})
