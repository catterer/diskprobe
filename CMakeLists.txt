cmake_minimum_required(VERSION 2.8)
project(diskprobe)

set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_STANDARD 17)

include_directories("${CMAKE_SOURCE_DIR}")

list(APPEND sources
    src/main.cc
    src/Probe.cc)

add_executable(dprobe ${sources})
target_link_libraries (dprobe pthread)
target_link_libraries (dprobe boost_system)
target_link_libraries (dprobe boost_program_options)
