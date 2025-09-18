enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    module_location_tests.cpp
    symbol_url_tests.cpp
    type_def_tests.cpp
    )

# define test suite driver

add_executable(lyric_common_testsuite ${TEST_CASES} ${TEST_MAIN_CPP})
target_link_libraries(lyric_common_testsuite PUBLIC
    lyric::lyric_common
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_common_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricCommonTestSuite OBJECT ${TEST_CASES})
target_link_libraries(LyricCommonTestSuite PUBLIC
    lyric::lyric_common
    tempo::tempo_test
    gtest::gtest
    )
