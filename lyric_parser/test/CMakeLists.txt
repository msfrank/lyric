enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASE_PREFIX lyric_parser)
set(TEST_CASES
    parse_archetype_tests.cpp
    parse_constant_tests.cpp
    )

# define test suite driver

set(TESTSUITE_EXECUTABLE "${TEST_CASE_PREFIX}_testsuite")
add_executable(${TESTSUITE_EXECUTABLE} ${TEST_CASES})
target_link_libraries(${TESTSUITE_EXECUTABLE} PUBLIC
    lyric::lyric_parser
    gtest::gtest
    )
gtest_discover_tests(${TESTSUITE_EXECUTABLE})

# define test suite static library

add_library(LyricParserTestSuite OBJECT ${TEST_CASES})
target_link_libraries(LyricParserTestSuite PUBLIC
    lyric::lyric_parser
    gtest::gtest
    )
