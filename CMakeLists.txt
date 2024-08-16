cmake_minimum_required(VERSION 3.14)
project(bench)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3 -msse2 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2 -mbmi")

include_directories(.)
include_directories(common)
include_directories(sketch)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_executable(bench
        common/Abstract.h
        common/bitset.h
        common/hash.h
        sketch/CM_BF.h
        sketch/CM_HT.h
        sketch/OO_PE.h
        benchmark.h
        main.cpp
        sketch/LF.h
        benchmark.h
)
