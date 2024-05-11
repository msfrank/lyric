enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    block_handle_tests.cpp
    )

# define test suite driver

add_executable(lyric_compiler_testsuite ${TEST_CASES})
target_link_libraries(lyric_compiler_testsuite PUBLIC
    lyric::lyric_compiler
    lyric::lyric_packaging
    gtest::gtest
)
gtest_discover_tests(lyric_compiler_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricCompilerTestSuite OBJECT ${TEST_CASES})
target_link_libraries(LyricCompilerTestSuite PUBLIC
    lyric::lyric_compiler
    lyric::lyric_packaging
    gtest::gtest
    )
