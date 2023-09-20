cmake_minimum_required(VERSION 3.21)

project(cord LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra -ggdb -fsanitize=undefined")

add_subdirectory(src)

add_subdirectory(tests)
add_subdirectory(examples)
