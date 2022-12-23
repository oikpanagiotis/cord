cmake_minimum_required(VERSION 3.10)

# This module contains basic non-domain specific structures and functions
# that are used throughout the rest of the library

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ggdb -fsanitize=undefined")
set(Sources
    error.c
    util.c
    array.c
    memory.c
)

add_library(http SHARED ${Sources})

target_include_directories(http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# install(TARGETS cord DESTINATION)
# install(FILES cord.h discord.h http.h DESTINATION /usr/local/include)