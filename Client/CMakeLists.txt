cmake_minimum_required(VERSION 3.16)
project(Client C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-Wall -Wextra -Werror)

add_executable(Client main.c client.c client.h)
