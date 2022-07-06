cmake_minimum_required(VERSION 3.8)

project ("perf_test")

set(CMAKE_CXX_STANDARD 17)

add_executable(perf_test main.cpp stats.h stats.cpp)

target_link_libraries(perf_test wiringPi pthread)