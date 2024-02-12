enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASE_PREFIX lyric_build)
set(TEST_CASES
    #builder_tests.cpp
    parse_module_task_tests.cpp
    sha256_hash_tests.cpp
    )

# define test suite driver

set(TESTSUITE_EXECUTABLE "${TEST_CASE_PREFIX}_testsuite")
add_executable(${TESTSUITE_EXECUTABLE} ${TEST_CASES})
target_link_libraries(${TESTSUITE_EXECUTABLE} PUBLIC
    lyric::lyric_build
    gtest::gtest
    )
gtest_discover_tests(${TESTSUITE_EXECUTABLE})

# define test suite static library

add_library(LyricBuildTestSuite OBJECT ${TEST_CASES})
target_link_libraries(LyricBuildTestSuite PUBLIC
    lyric::lyric_build
    gtest::gtest
   )
