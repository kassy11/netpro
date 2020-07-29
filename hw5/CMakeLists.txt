cmake_minimum_required(VERSION 3.16)
project(hw5 C)

set(CMAKE_C_STANDARD 99)

add_executable(hw5 idobata.c idobata_client.c idobata_server.c idobata.h idobata_util.c)

include_directories(../mynet)