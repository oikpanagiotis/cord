cmake_minimum_required(VERSION 3.10)

# This module contains basic non-domain specific structures and functions
# that are used throughout the rest of the library

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ggdb -fsanitize=undefined")
set(Sources
    array.c
    error.c
    memory.c
    util.c
    string.c
)

add_library(core SHARED ${Sources})

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})