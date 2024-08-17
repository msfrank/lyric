enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    analyze_block_tests.cpp
    analyze_class_tests.cpp
    analyze_concept_tests.cpp
    analyze_enum_tests.cpp
    analyze_function_tests.cpp
    analyze_instance_tests.cpp
    analyze_namespace_tests.cpp
    analyze_static_tests.cpp
    analyze_struct_tests.cpp
    )

# define test suite driver

add_executable(lyric_analyzer_testsuite ${TEST_CASES})
target_compile_definitions(lyric_analyzer_testsuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_analyzer_testsuite PUBLIC
    lyric::lyric_analyzer
    lyric::lyric_bootstrap
    lyric::lyric_build
    lyric::lyric_schema
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_analyzer_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricAnalyzerTestSuite OBJECT ${TEST_CASES})
target_compile_definitions(LyricAnalyzerTestSuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricAnalyzerTestSuite PUBLIC
    lyric::lyric_analyzer
    lyric::lyric_bootstrap
    lyric::lyric_build
    lyric::lyric_schema
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
