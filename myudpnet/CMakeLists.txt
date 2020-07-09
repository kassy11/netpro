cmake_minimum_required(VERSION 3.16)
project(myudpnet C)

set(CMAKE_C_STANDARD 99)

add_library(myudpnet init_udpclient.c init_udpserver.c myudpnet.h)
include_directories(../mynet)