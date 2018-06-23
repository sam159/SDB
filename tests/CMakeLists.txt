include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

find_package(Check REQUIRED)
find_package (Threads REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

set(TEST_SOURCES
        check_sdb.c
        bplus_tree_test.c bplus_tree_test.h)

add_executable(check_SDB ${TEST_SOURCES})
target_link_libraries(check_SDB SDBLib ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})