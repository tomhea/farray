cmake_minimum_required(VERSION 3.17)
project(Farray)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -O2)

add_executable(tests tests/tests_farray1.cpp)
add_executable(times timings/times_farray1.cpp)
