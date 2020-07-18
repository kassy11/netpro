cmake_minimum_required(VERSION 3.16)
project(broadcast_udp C)

set(CMAKE_C_STANDARD 99)

add_executable(broadcast_udp main.c echo_client.c)

include_directories(../mynet)