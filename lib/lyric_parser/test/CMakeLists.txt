enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    load_archetype_tests.cpp
    parse_archetype_tests.cpp
    parse_constant_tests.cpp
    parse_macro_tests.cpp
    parse_val_statement_tests.cpp
    )

# define test suite driver

add_executable(lyric_parser_testsuite ${TEST_CASES})
target_link_libraries(lyric_parser_testsuite PUBLIC
    lyric::lyric_parser
    lyric::lyric_schema
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_parser_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricParserTestSuite OBJECT ${TEST_CASES})
target_link_libraries(LyricParserTestSuite PUBLIC
    lyric::lyric_parser
    lyric::lyric_schema
    tempo::tempo_test
    gtest::gtest
    )
