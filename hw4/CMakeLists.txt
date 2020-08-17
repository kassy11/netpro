cmake_minimum_required(VERSION 3.16)
project(hw4 C)

set(CMAKE_C_STANDARD 99)

add_executable(hw4 chat_client.c chat_server.c chat_utils.c chat.h chat.c)

include_directories(../mynet)