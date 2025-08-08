enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    symbolize_block_tests.cpp
    symbolize_def_tests.cpp
    symbolize_defclass_tests.cpp
    symbolize_defconcept_tests.cpp
    symbolize_namespace_tests.cpp
    symbolize_static_tests.cpp
    symbolize_typename_tests.cpp
    )

# define test suite driver

add_executable(lyric_symbolizer_testsuite
    ${TEST_CASES}
    base_symbolizer_fixture.cpp base_symbolizer_fixture.h
    ${TEST_MAIN_CPP}
    )
target_compile_definitions(lyric_symbolizer_testsuite PRIVATE
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_symbolizer_testsuite PUBLIC
    lyric::lyric_build
    lyric::lyric_bootstrap
    lyric::lyric_schema
    lyric::lyric_symbolizer
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_symbolizer_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricSymbolizerTestSuite OBJECT ${TEST_CASES} base_symbolizer_fixture.cpp base_symbolizer_fixture.h)
target_compile_definitions(LyricSymbolizerTestSuite PRIVATE
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricSymbolizerTestSuite PUBLIC
    lyric::lyric_build
    lyric::lyric_bootstrap
    lyric::lyric_schema
    lyric::lyric_symbolizer
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
