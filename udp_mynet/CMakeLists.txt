cmake_minimum_required(VERSION 3.16)
project(udp_mynet C)

set(CMAKE_C_STANDARD 99)

add_executable(udp_mynet echo_client.c echo_server.c)

include_directories(../mynet)