enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    compiler_scan_driver_tests.cpp
    compile_assignment_tests.cpp
    compile_conditional_tests.cpp
    compile_data_deref_tests.cpp
    compile_def_tests.cpp
    compile_defalias_tests.cpp
    compile_defclass_tests.cpp
    compile_defconcept_tests.cpp
    compile_defenum_tests.cpp
    compile_definstance_tests.cpp
    compile_defstruct_tests.cpp
    compile_expect_tests.cpp
    compile_import_tests.cpp
    compile_lambda_tests.cpp
    compile_load_and_store_macro_tests.cpp
    compile_match_tests.cpp
    compile_namespace_tests.cpp
    compile_new_tests.cpp
    compile_typename_tests.cpp
    compile_val_tests.cpp
    compile_var_tests.cpp
    compile_while_tests.cpp
    visitor_context_tests.cpp
    )

# define test suite driver

add_executable(lyric_compiler_testsuite
    ${TEST_CASES}
    base_compiler_fixture.cpp base_compiler_fixture.h
    compiler_mocks.h
    ${TEST_MAIN_CPP}
    )
target_compile_definitions(lyric_compiler_testsuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(lyric_compiler_testsuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_compiler
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
gtest_discover_tests(lyric_compiler_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricCompilerTestSuite OBJECT
    ${TEST_CASES}
    base_compiler_fixture.cpp base_compiler_fixture.h
    compiler_mocks.h
    )
target_compile_definitions(LyricCompilerTestSuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    )
target_link_libraries(LyricCompilerTestSuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_compiler
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )
