cmake_minimum_required(VERSION 3.16)
project(mynet C)

set(CMAKE_C_STANDARD 99)

add_library(mynet library.c library.h init_tcpserver.c init_tcpclient.c mynet.h other.c)
