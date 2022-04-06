cmake_minimum_required(VERSION 3.16)
project(Server C)

set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall -Wextra -Werror)
add_compile_options(-pthread)

add_executable(Server main.c server.c server.h)
