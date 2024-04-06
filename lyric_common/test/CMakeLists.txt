enable_testing()

include(GoogleTest)

# define unit tests

set(LYRIC_COMMON_TEST_CASES
    symbol_url_tests.cpp
    )

# define test suite driver

add_executable(lyric_common_testsuite ${LYRIC_COMMON_TEST_CASES})
target_link_libraries(lyric_common_testsuite PUBLIC
    lyric::lyric_common
    gtest::gtest
    )
gtest_discover_tests(lyric_common_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricCommonTestSuite OBJECT ${LYRIC_COMMON_TEST_CASES})
target_link_libraries(LyricCommonTestSuite PUBLIC
    lyric::lyric_common
    gtest::gtest
    )
