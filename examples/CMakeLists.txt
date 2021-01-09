cmake_minimum_required(VERSION 3.10)

set(Libraries
    crypto
    ev
    uwsc
    jansson
    curl
    cord
)

add_executable(ping_pong ping_pong.c)
target_link_libraries(ping_pong PUBLIC ${Libraries})

add_executable(simple simple.c)
target_link_libraries(simple PUBLIC ${Libraries})