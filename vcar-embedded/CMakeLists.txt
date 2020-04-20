cmake_minimum_required(VERSION 3.0)
project(vcar)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

file(GLOB CXX_EXEC
        src/*.cpp
        inc/*.c
        inc/linux/can.c)

add_library(vcar SHARED ${CXX_EXEC})
target_include_directories(vcar PUBLIC inc/)
