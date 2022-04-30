cmake_minimum_required(VERSION 3.10)

add_executable(json_tests json_tests.c)

add_test(NAME json COMMAND json_tests)
