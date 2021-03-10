cmake_minimum_required(VERSION 3.10)

set(Libraries
    crypto
    ev
    uwsc
    jansson
    curl
    cord
)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ggdb -fsanitize=undefined")
add_executable(ping_pong ping_pong.c)
target_link_libraries(ping_pong PUBLIC ${Libraries})