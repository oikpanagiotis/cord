cmake_minimum_required(VERSION 3.10)

# This module contains basic non-domain specific structures and functions
# that are used throughout the rest of the library

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ggdb -fsanitize=undefined")
set(Sources
    discord.c
    events.c
    types.c
)

add_library(discord SHARED ${Sources})

target_include_directories(discord PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})