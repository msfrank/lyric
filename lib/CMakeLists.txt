
# define the test_main.cpp in a single place for all test suites
set(TEST_MAIN_CPP ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp)


#######################################################################
#
# add targets from subdirectories
#
#######################################################################

add_subdirectory(lyric_analyzer)
add_subdirectory(lyric_archiver)
add_subdirectory(lyric_assembler)
add_subdirectory(lyric_bootstrap)
add_subdirectory(lyric_build)
add_subdirectory(lyric_common)
add_subdirectory(lyric_compiler)
add_subdirectory(lyric_importer)
add_subdirectory(lyric_object)
add_subdirectory(lyric_optimizer)
add_subdirectory(lyric_parser)
add_subdirectory(lyric_rewriter)
add_subdirectory(lyric_runtime)
add_subdirectory(lyric_schema)
add_subdirectory(lyric_serde)
add_subdirectory(lyric_symbolizer)
add_subdirectory(lyric_test)
add_subdirectory(lyric_typing)


#######################################################################
#
# declare the package test suite target
#
#######################################################################

add_executable(lyric_testsuite ${TEST_MAIN_CPP})
target_link_libraries(lyric_testsuite
    PUBLIC gtest::gtest
    PRIVATE
    LyricAnalyzerTestSuite
    LyricArchiverTestSuite
    LyricAssemblerTestSuite
    LyricBootstrapTestSuite
    LyricBuildTestSuite
    LyricCommonTestSuite
    LyricCompilerTestSuite
    LyricObjectTestSuite
    LyricOptimizerTestSuite
    LyricParserTestSuite
    LyricRewriterTestSuite
    LyricSymbolizerTestSuite
    LyricTypingTestSuite
)
gtest_discover_tests(lyric_testsuite DISCOVERY_TIMEOUT 30)
