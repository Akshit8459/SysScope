file(GLOB_RECURSE TEST_SOURCES
    "unit/*.cpp"
    "integration/*.cpp"
)

add_executable(syscope_test_suite
    test_main.cpp
    ${TEST_SOURCES}
)

target_include_directories(syscope_test_suite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(syscope_test_suite PRIVATE sysscope_core)
set_target_compiler_warnings(syscope_test_suite)

add_test(NAME SysScopeTestSuite COMMAND syscope_test_suite)
