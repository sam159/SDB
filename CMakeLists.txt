cmake_minimum_required(VERSION 3.10)
project(SDB C)

set(CMAKE_C_STANDARD 11)

add_executable(SDB src/main.c src/InputBuffer.c src/InputBuffer.h src/SQL.c src/SQL.h)