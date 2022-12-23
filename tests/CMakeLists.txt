cmake_minimum_required(VERSION 3.10)

set(Libraries
    crypto
    ev
    uwsc
    jansson
    curl

    core
)

add_executable(json_tests json_tests.c)
target_link_libraries(json_tests PUBLIC ${Libraries})
add_test(NAME test_json COMMAND json)

add_executable(container_tests container_tests.c)
target_link_libraries(container_tests PUBLIC ${Libraries})
add_test(NAME test_container COMMAND container)

add_executable(array_tests array_tests.c)
target_link_libraries(array_tests PUBLIC ${Libraries})
add_test(NAME test_array COMMAND array)

add_custom_target(test_all
    COMMAND ./json_tests
    COMMAND ./container_tests
    COMMAND ./array_tests
)

add_custom_target(test_report
    COMMAND rm -f test_report.txt
    COMMAND ./json_tests >> test_report.txt
    COMMAND ./container_tests >> test_report.txt
    COMMAND ./array_tests >> test_report.txt
)