cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ggdb -fsanitize=undefined")
set(Sources
    cord.c
    discord.c
    events.c
    http.c
    types.c
    sds.c
)

set(Libraries
    crypto
    ev
    uwsc
    jansson
    curl

    core
)

add_subdirectory(core)
# add_subdirectory(discord)
# add_subdirectory(http)

add_library(cord SHARED ${Sources})

target_link_libraries(cord PUBLIC ${Libraries})
target_include_directories(cord PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

install(TARGETS cord DESTINATION)
install(FILES cord.h discord.h http.h DESTINATION /usr/local/include)