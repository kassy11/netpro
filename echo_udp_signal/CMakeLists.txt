cmake_minimum_required(VERSION 3.16)
project(echo_udp_signal C)

set(CMAKE_C_STANDARD 99)

add_executable(echo_udp_signal echo_client.c echo_server.c)

include_directories(../mynet)