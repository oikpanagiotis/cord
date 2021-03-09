cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ggdb")
set(Sources
    cord.c
    discord.c
    error.c
    events.c
    http.c
    types.c
    util.c
    sds.c
    array.c
)

set(Libraries
    crypto
    ev
    uwsc
    jansson
    curl
)


add_library(cord SHARED ${Sources})

target_link_libraries(cord PUBLIC ${Libraries})
target_include_directories(cord PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS cord DESTINATION)
install(FILES cord.h discord.h http.h DESTINATION /usr/local/include)