cmake_minimum_required(VERSION 3.21)

# This module contains basic non-domain specific structures and functions
# that are used throughout the rest of the library

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(Sources
    array.c
    errors.c
    memory.c
    strings.c
    log.c
    hashmap.c
)

add_library(core SHARED ${Sources})

target_include_directories(core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
