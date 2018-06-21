cmake_minimum_required(VERSION 3.10)
project(SDB C)

set(CMAKE_C_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/lib)
add_executable(SDB src/main.c src/InputBuffer.c src/InputBuffer.h src/SQL.c src/SQL.h src/scanner.c src/scanner.h src/parser.c src/parser.h src/bplus_tree.c src/bplus_tree.h)
