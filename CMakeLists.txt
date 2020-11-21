cmake_minimum_required(VERSION 3.17)
project(O1)

set(CMAKE_CXX_STANDARD 14)

add_executable(O1 tests.cpp o1array.h)
add_executable(O1bit tests1bit.cpp o1array1bit.h)
