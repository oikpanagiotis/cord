cmake_minimum_required(VERSION 3.10)

set(Libraries 
    cord
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

add_executable(ping_pong ping_pong.c)

target_include_directories(ping_pong PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ping_pong PUBLIC ${Libraries})
