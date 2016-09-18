cmake_minimum_required(VERSION 3.3)
project(OPMORL_2_0)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -g")

set(SOURCE_FILES
    init.c
    io.c
    lib.c
    map.c
    monster.c
    object.c
    opmorl.c
    opmorl.h
        player.c linkedlist.c linkedlist.h turn.c)

add_executable(opmorl ${SOURCE_FILES})
target_link_libraries(opmorl ${CURSES_LIBRARIES})