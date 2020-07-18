cmake_minimum_required(VERSION 3.16)
project(echo_client2 C)

set(CMAKE_C_STANDARD 99)

add_executable(echo_client2 echo_client2.c echo_server2.c echo_client_udp.c echo_server_udp.c)

include_directories(../mynet)
