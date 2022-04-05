cmake_minimum_required(VERSION 3.21)
project(orderedcodepp)

add_subdirectory(libs/Catch2)

set(CMAKE_CXX_STANDARD 20)

add_executable(orderedcode_test tests/orderedcode_test.cpp)
target_link_libraries(orderedcode_test Catch2WithMain)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
