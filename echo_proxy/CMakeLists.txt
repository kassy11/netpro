cmake_minimum_required(VERSION 3.16)
project(tcp_echo C)

set(CMAKE_C_STANDARD 99)

add_executable(tcp_echo main.c echo_client.c)