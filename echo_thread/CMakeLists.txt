cmake_minimum_required(VERSION 3.16)
project(echo_thread C)

set(CMAKE_C_STANDARD 99)

add_executable(echo_thread echo_server_thread.c)

include_directories(../mynet)