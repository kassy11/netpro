cmake_minimum_required(VERSION 3.16)
project(tcp_server C)

set(CMAKE_C_STANDARD 99)

add_executable(tcp_server main.c tcp_server.c)