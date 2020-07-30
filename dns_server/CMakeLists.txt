cmake_minimum_required(VERSION 3.16)
project(dns_server C)

set(CMAKE_C_STANDARD 99)

add_executable(dns_server query_host.c dnshead.h)

include_directories(../mynet)