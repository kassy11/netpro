cmake_minimum_required(VERSION 3.16)
project(quiz_select C)

set(CMAKE_C_STANDARD 99)

add_executable(quiz_select quiz.c quiz_client.c quiz.h quiz_server.c quiz_util.c question.c)

include_directories(../mynet)