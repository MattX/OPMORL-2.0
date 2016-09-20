cmake_minimum_required(VERSION 3.3)
project(OPMORL_2_0)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -g")

set(SOURCE_FILES
        src/init.c
        src/io.c
        src/lib.c
        src/map.c
        src/monster.c
        src/object.c
        src/opmorl.c
        src/opmorl.h
        src/player.c
        src/linkedlist.c
        src/linkedlist.h
        src/turn.c
        src/inv.c)

add_executable(opmorl ${SOURCE_FILES})
target_link_libraries(opmorl ${CURSES_LIBRARIES})