enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    rewrite_trap_tests.cpp
    )

# define test suite driver

add_executable(lyric_rewriter_testsuite ${TEST_CASES})
target_compile_definitions(lyric_rewriter_testsuite PRIVATE
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_rewriter_testsuite PUBLIC
    lyric::lyric_build
    lyric::lyric_bootstrap
    lyric::lyric_rewriter
    lyric::lyric_schema
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_rewriter_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricRewriterTestSuite OBJECT ${TEST_CASES})
target_compile_definitions(LyricRewriterTestSuite PRIVATE
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricRewriterTestSuite PUBLIC
    lyric::lyric_build
    lyric::lyric_bootstrap
    lyric::lyric_rewriter
    lyric::lyric_schema
    tempo::tempo_test
    gtest::gtest
    )
