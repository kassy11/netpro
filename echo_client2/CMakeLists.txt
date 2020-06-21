cmake_minimum_required(VERSION 3.16)
project(echo_client2 C)

set(CMAKE_C_STANDARD 99)

add_executable(echo_client2 main.c echo_client2.c)

include_directories(../mynet)
