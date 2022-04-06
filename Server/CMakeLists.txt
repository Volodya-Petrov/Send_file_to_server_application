cmake_minimum_required(VERSION 3.16)
project(Server C)

set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall -Wextra -Werror)
set(CMAKE_C_FLAGS -pthread)

add_executable(Server main.c server.c server.h)
