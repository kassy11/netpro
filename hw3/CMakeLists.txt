cmake_minimum_required(VERSION 3.16)
project(hw3 C)

set(CMAKE_C_STANDARD 99)

add_executable(hw3 echofork_improve.c echo_fork_thread.c echothread_improve.c)

include_directories(../mynet)