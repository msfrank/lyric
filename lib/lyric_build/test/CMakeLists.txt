enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    #builder_tests.cpp
    build_runner_tests.cpp
    compile_plugin_task_tests.cpp
    fetch_external_file_task_tests.cpp
    parse_module_task_tests.cpp
    provide_plugin_task_tests.cpp
    )

# define test suite driver

add_executable(lyric_build_testsuite ${TEST_CASES}
    base_build_fixture.cpp
    base_build_fixture.h
    test_task.cpp
    test_task.h
    )
target_link_libraries(lyric_build_testsuite PUBLIC
    lyric::lyric_build
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_build_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricBuildTestSuite OBJECT ${TEST_CASES}
    base_build_fixture.cpp
    base_build_fixture.h
    test_task.cpp
    test_task.h
    )
target_link_libraries(LyricBuildTestSuite PUBLIC
    lyric::lyric_build
    tempo::tempo_test
    gtest::gtest
   )
