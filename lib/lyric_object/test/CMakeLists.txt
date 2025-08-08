enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    type_walker_tests.cpp
    )

# define test suite driver

add_executable(lyric_object_testsuite ${TEST_CASES} ${TEST_MAIN_CPP})
target_link_libraries(lyric_object_testsuite PUBLIC
    lyric::lyric_object
    flatbuffers::flatbuffers
    gtest::gtest
    )
gtest_discover_tests(lyric_object_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricObjectTestSuite OBJECT ${TEST_CASES})
target_link_libraries(LyricObjectTestSuite PUBLIC
    lyric::lyric_object
    flatbuffers::flatbuffers
    gtest::gtest
    )
