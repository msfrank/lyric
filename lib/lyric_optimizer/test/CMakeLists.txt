enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    build_proc_tests.cpp
    parse_proc_tests.cpp
    )

# define test suite driver

add_executable(lyric_optimizer_testsuite
    ${TEST_CASES}
    base_optimizer_fixture.cpp base_optimizer_fixture.h
    )
target_compile_definitions(lyric_optimizer_testsuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_optimizer_testsuite PUBLIC
    lyric::lyric_optimizer
    lyric::lyric_bootstrap
    lyric::lyric_build
    lyric::lyric_runtime
    lyric::lyric_schema
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_optimizer_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricOptimizerTestSuite OBJECT
    ${TEST_CASES}
    base_optimizer_fixture.cpp base_optimizer_fixture.h
    )
target_compile_definitions(LyricOptimizerTestSuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricOptimizerTestSuite PUBLIC
    lyric::lyric_optimizer
    lyric::lyric_bootstrap
    lyric::lyric_build
    lyric::lyric_runtime
    lyric::lyric_schema
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
