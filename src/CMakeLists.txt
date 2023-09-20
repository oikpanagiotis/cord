cmake_minimum_required(VERSION 3.21)

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

add_subdirectory(cord)
add_subdirectory(core)
add_subdirectory(discord)
add_subdirectory(http)

target_include_directories(cord PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/discord
    ${CMAKE_CURRENT_SOURCE_DIR}/http
)

install(TARGETS cord DESTINATION)
install(FILES cord/cord.h DESTINATION /usr/local/include)
