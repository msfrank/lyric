enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    block_handle_tests.cpp
    code_fragment_tests.cpp
    )

# define test suite driver

add_executable(lyric_assembler_testsuite ${TEST_CASES})
target_compile_definitions(lyric_assembler_testsuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_assembler_testsuite PUBLIC
    lyric::lyric_assembler
    lyric::lyric_bootstrap
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_assembler_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricAssemblerTestSuite OBJECT ${TEST_CASES})
target_compile_definitions(lyric_assembler_testsuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricAssemblerTestSuite PUBLIC
    lyric::lyric_assembler
    lyric::lyric_bootstrap
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
