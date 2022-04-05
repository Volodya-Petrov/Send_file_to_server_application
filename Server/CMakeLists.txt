cmake_minimum_required(VERSION 3.21)
project(Server C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -pthread)
add_executable(Server main.c server.c server.h)
