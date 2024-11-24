enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    archive_call_tests.cpp
    )

# define test suite driver

add_executable(lyric_archiver_testsuite ${TEST_CASES})
target_compile_definitions(lyric_archiver_testsuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_archiver_testsuite PUBLIC
    lyric::lyric_archiver
    lyric::lyric_bootstrap
    lyric::lyric_build
    lyric::lyric_schema
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_archiver_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricArchiverTestSuite OBJECT ${TEST_CASES})
target_compile_definitions(LyricArchiverTestSuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricArchiverTestSuite PUBLIC
    lyric::lyric_archiver
    lyric::lyric_bootstrap
    lyric::lyric_build
    lyric::lyric_schema
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
