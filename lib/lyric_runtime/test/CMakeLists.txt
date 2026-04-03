enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    connection_tests.cpp
    port_multiplexer_tests.cpp
    )

# define test suite driver

add_executable(lyric_runtime_testsuite
    ${TEST_CASES}
    base_runtime_fixture.cpp base_runtime_fixture.h
    runtime_mocks.h
    )
target_compile_definitions(lyric_runtime_testsuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_runtime_testsuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_runtime
    lyric::lyric_test
    tempo::tempo_test
    tempo::tempo_test_main
    gtest::gtest
    )
gtest_discover_tests(lyric_runtime_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricRuntimeTestSuite OBJECT
    ${TEST_CASES}
    base_runtime_fixture.cpp base_runtime_fixture.h
    runtime_mocks.h
    )
target_compile_definitions(LyricRuntimeTestSuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricRuntimeTestSuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_runtime
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
