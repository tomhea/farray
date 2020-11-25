cmake_minimum_required(VERSION 3.17)
project(O1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -O2)

add_executable(tests tests/tests.cpp)
add_executable(times tests/times.cpp)
