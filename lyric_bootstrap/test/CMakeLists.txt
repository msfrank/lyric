enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASE_PREFIX lyric_bootstrap)
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
    Uri_tests.cpp
    Utf16_tests.cpp
    val_tests.cpp
    var_tests.cpp
    while_tests.cpp
    )

# define test suite driver

set(TESTSUITE_EXECUTABLE "${TEST_CASE_PREFIX}_testsuite")
add_executable(${TESTSUITE_EXECUTABLE} ${TEST_CASES} test_helpers.cpp test_helpers.h)
#add_dependencies(${TESTSUITE_EXECUTABLE} lyric_bootstrap)
target_link_libraries(${TESTSUITE_EXECUTABLE} PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_test
    gtest::gtest
    )
gtest_discover_tests(${TESTSUITE_EXECUTABLE})

# define test suite static library

add_library(LyricBootstrapTestSuite OBJECT ${TEST_CASES} test_helpers.cpp test_helpers.h)
target_link_libraries(LyricBootstrapTestSuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_test
    gtest::gtest
    )
