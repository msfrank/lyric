
enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASE_PREFIX lyric_packaging)
set(TEST_CASES
    #package_loader_tests.cpp
    package_reader_tests.cpp
    package_specifier_tests.cpp
    package_writer_tests.cpp
    )

# define test suite driver

set(TESTSUITE_EXECUTABLE "${TEST_CASE_PREFIX}_testsuite")
add_executable(${TESTSUITE_EXECUTABLE} ${TEST_CASES})
target_include_directories(${TESTSUITE_EXECUTABLE} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(${TESTSUITE_EXECUTABLE} PUBLIC
    lyric::lyric_packaging
    gtest::gtest
    )
gtest_discover_tests(${TESTSUITE_EXECUTABLE})

# define test suite static library

add_library(LyricPackagingTestSuite OBJECT ${TEST_CASES})
target_include_directories(LyricPackagingTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(LyricPackagingTestSuite PUBLIC
    lyric::lyric_packaging
    gtest::gtest
    )
