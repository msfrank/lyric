enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    assignment_tests.cpp
    Bool_tests.cpp
    Char_tests.cpp
    conditional_tests.cpp
    def_tests.cpp
    defclass_tests.cpp
    defconcept_tests.cpp
    defenum_tests.cpp
    definstance_tests.cpp
    defstruct_tests.cpp
    Float_tests.cpp
    Int_tests.cpp
    Iterator_tests.cpp
    lambda_tests.cpp
    Map_tests.cpp
    match_tests.cpp
    namespace_tests.cpp
    new_tests.cpp
    Option_tests.cpp
    Pair_tests.cpp
    Seq_tests.cpp
    Status_tests.cpp
    String_tests.cpp
    Tuple_tests.cpp
    Url_tests.cpp
    Utf16_tests.cpp
    val_tests.cpp
    var_tests.cpp
    while_tests.cpp
    )

# define test suite driver

add_executable(lyric_bootstrap_testsuite ${TEST_CASES} test_helpers.cpp test_helpers.h)
target_compile_definitions(lyric_bootstrap_testsuite PRIVATE
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
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricBootstrapTestSuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_test
    gtest::gtest
    )
