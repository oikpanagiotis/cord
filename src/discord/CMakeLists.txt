cmake_minimum_required(VERSION 3.10)

# This module contains basic non-domain specific structures and functions
# that are used throughout the rest of the library

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ggdb")
set(Sources
    client.c
    events.c
    types.c
    entities.c
    serialization.c
)

set(Libraries
    core
    http
)


add_library(discord SHARED ${Sources})


target_link_libraries(discord PUBLIC ${Libraries})
target_include_directories(discord PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})