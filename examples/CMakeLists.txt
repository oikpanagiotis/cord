cmake_minimum_required(VERSION 3.10)

set(Libraries
    crypto
    ev
    uwsc
    jansson
    curl
    cord
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

add_executable(ping_pong ping_pong.c)
add_executable(simple simple.c)

target_include_directories(ping_pong PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
)
target_include_directories(simple PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
)

target_link_libraries(simple PUBLIC ${Libraries})
target_link_libraries(ping_pong PUBLIC ${Libraries})
