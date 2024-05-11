enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    callsite_reifier_tests.cpp
    callsite_reifier_tp0_in_tests.cpp
    callsite_reifier_tp0_inout_tests.cpp
    callsite_reifier_tp0_out_tests.cpp
    compare_assignable_tests.cpp
    is_assignable_tests.cpp
    )

# define test suite driver

add_executable(lyric_typing_testsuite ${TEST_CASES} base_fixture.cpp base_fixture.h)
target_compile_definitions(lyric_typing_testsuite PRIVATE
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_typing_testsuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_typing
    gtest::gtest
    )
gtest_discover_tests(lyric_typing_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricTypingTestSuite OBJECT ${TEST_CASES} base_fixture.cpp base_fixture.h)
target_compile_definitions(LyricTypingTestSuite PRIVATE
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricTypingTestSuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_typing
    gtest::gtest
    )
