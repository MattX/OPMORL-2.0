cmake_minimum_required(VERSION 3.3)
project(OPMORL_2_0)

include(CMakeLocal.txt OPTIONAL)

if(WIN32)
    set(PATH "C:\\MinGW")
endif()

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -Wall")
set(CMAKE_C_COMPILER "/usr/local/Cellar/gcc/6.3.0/bin/gcc-6")

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
        src/lib.h
        src/turn.c
        src/inv.c
        src/use.c)

add_executable(opmorl ${SOURCE_FILES})
target_link_libraries(opmorl ${CURSES_LIBRARIES} m)
