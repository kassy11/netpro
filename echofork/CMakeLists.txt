cmake_minimum_required(VERSION 3.16)
project(echofork C)

set(CMAKE_C_STANDARD 99)

add_executable(echofork main.c echofork.c)

include_directories(../mynet)