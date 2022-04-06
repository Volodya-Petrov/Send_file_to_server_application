cmake_minimum_required(VERSION 3.16)
project(Client_Server C)

set(CMAKE_C_STANDART 99)
set(CMAKE_C_FLAGS -pthread)
add_library(${PROJECT_NAME} Server/server.c
	Server/server.h
	Client/client.c
	Client/client.h
	)
add_executable(Server Server/main.c)
target_link_libraries(Server ${PROJECT_NAME})
add_executable(Client Client/main.c)
target_link_libraries(Client ${PROJECT_NAME})

