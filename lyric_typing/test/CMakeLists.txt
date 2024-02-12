enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASE_PREFIX lyric_typing)
set(TEST_CASES
    callsite_reifier_tests.cpp
    )

# define test suite driver

set(TESTSUITE_EXECUTABLE "${TEST_CASE_PREFIX}_testsuite")
add_executable(${TESTSUITE_EXECUTABLE} ${TEST_CASES})
target_link_libraries(${TESTSUITE_EXECUTABLE} PUBLIC
    lyric::lyric_typing
    gtest::gtest
    )
gtest_discover_tests(${TESTSUITE_EXECUTABLE})

# define test suite static library

add_library(LyricTypingTestSuite OBJECT ${TEST_CASES})
target_link_libraries(LyricTypingTestSuite PUBLIC
    lyric::lyric_typing
    gtest::gtest
    )
