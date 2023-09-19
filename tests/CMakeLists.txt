cmake_minimum_required(VERSION 3.10)

set(CoreModuleLibraries
    core
)

add_executable(json_tests json_tests.c)
target_link_libraries(json_tests ${CoreModuleLibraries})
add_test(NAME test_json COMMAND json)

add_executable(container_tests container_tests.c)
target_link_libraries(container_tests ${CoreModuleLibraries})
add_test(NAME test_container COMMAND container)

add_executable(allocators_tests allocators_tests.c)
target_link_libraries(allocators_tests ${CoreModuleLibraries})
add_test(NAME test_allocators COMMAND allocators)

add_executable(string_tests string_tests.c)
target_link_libraries(string_tests ${CoreModuleLibraries})
add_test(NAME test_string COMMAND string)

add_custom_target(test_all
    COMMAND ./json_tests
    COMMAND ./container_tests
    COMMAND ./allocators_tests
    COMMAND ./string_tests
)

add_custom_target(test_report
    COMMAND rm -f test_report.txt
    COMMAND ./json_tests >> test_report.txt
    COMMAND ./container_tests >> test_report.txt
    COMMAND ./allocators_tests >> test_report.txt
    COMMAND ./string_tests >> test_report.txt
)
