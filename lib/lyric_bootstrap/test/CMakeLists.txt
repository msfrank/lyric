enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    Bool_tests.cpp
    Bytes_tests.cpp
    Char_tests.cpp
    Float_tests.cpp
    Int_tests.cpp
    Iterator_tests.cpp
    Map_tests.cpp
    Nil_tests.cpp
    Option_tests.cpp
    Pair_tests.cpp
    Seq_tests.cpp
    Status_tests.cpp
    String_tests.cpp
    Tuple_tests.cpp
    Type_tests.cpp
    Undef_tests.cpp
    Url_tests.cpp
    Utf16_tests.cpp
    )

# define test suite driver

add_executable(lyric_bootstrap_testsuite ${TEST_CASES} test_helpers.cpp test_helpers.h)
target_compile_definitions(lyric_bootstrap_testsuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_bootstrap_testsuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_test
    gtest::gtest
    )
gtest_discover_tests(lyric_bootstrap_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricBootstrapTestSuite OBJECT ${TEST_CASES} test_helpers.cpp test_helpers.h)
target_compile_definitions(LyricBootstrapTestSuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricBootstrapTestSuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_test
    gtest::gtest
    )
