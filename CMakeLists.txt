cmake_minimum_required(VERSION 3.10)

project(cord)

# Try removing set CMAKE_C_FLAGS in other CmakeLists.txt files

# Find out what needs to be done to se this to C11
# Transition to C11
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ggdb -fsanitize=undefined")

add_subdirectory(src)

# Removed temporarily while refactoring
add_subdirectory(tests)
add_subdirectory(examples)
