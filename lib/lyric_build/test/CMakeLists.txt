enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    #builder_tests.cpp
    parse_module_task_tests.cpp
    sha256_hash_tests.cpp
    )

# define test suite driver

add_executable(lyric_build_testsuite ${TEST_CASES})
target_link_libraries(lyric_build_testsuite PUBLIC
    lyric::lyric_build
    gtest::gtest
    )
gtest_discover_tests(lyric_build_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricBuildTestSuite OBJECT ${TEST_CASES})
target_link_libraries(LyricBuildTestSuite PUBLIC
    lyric::lyric_build
    gtest::gtest
   )
