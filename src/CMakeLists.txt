
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_library(SDBLib STATIC
        InputBuffer.c InputBuffer.h
        SQL.c SQL.h
        scanner.c scanner.h
        parser.c parser.h
        bplus_tree.c bplus_tree.h
        ../lib/MurmurHash3.c ../lib/MurmurHash3.h random.c random.h Table.c Table.h)

add_executable(SDB main.c)
target_link_libraries(SDB SDBLib)