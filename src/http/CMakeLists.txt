cmake_minimum_required(VERSION 3.21)

# This module contains basic non-domain specific structures and functions
# that are used throughout the rest of the library

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(Sources
    http.c
)

set(Libraries
    curl
)

add_library(http SHARED ${Sources})

target_link_libraries(http PUBLIC ${Libraries})

target_include_directories(http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
