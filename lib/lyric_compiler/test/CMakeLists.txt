enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    compile_typename_tests.cpp
    compiler_scan_driver_tests.cpp
    visitor_context_tests.cpp
    )

# define test suite driver

add_executable(lyric_compiler_testsuite
    ${TEST_CASES}
    base_compiler_fixture.cpp base_compiler_fixture.h
    compiler_mocks.h
    )
target_compile_definitions(lyric_compiler_testsuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_compiler_testsuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_compiler
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_compiler_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricCompilerTestSuite OBJECT
    ${TEST_CASES}
    base_compiler_fixture.cpp base_compiler_fixture.h
    compiler_mocks.h
    )
target_compile_definitions(LyricCompilerTestSuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricCompilerTestSuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_compiler
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
