cmake_minimum_required(VERSION 3.21)

set(Sources
    cord.c
)

set(Libraries
    crypto
    ev
    uwsc
    jansson
    curl

    core
    discord
    http
)

add_library(cord SHARED ${Sources})

target_link_libraries(cord PUBLIC ${Libraries})

target_include_directories(cord PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../discord
    ${CMAKE_CURRENT_SOURCE_DIR}/../http
)

install(TARGETS cord DESTINATION)
install(FILES cord.h DESTINATION /usr/local/include)
