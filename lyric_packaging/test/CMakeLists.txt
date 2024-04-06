
enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    #package_loader_tests.cpp
    package_reader_tests.cpp
    package_specifier_tests.cpp
    package_writer_tests.cpp
    )

# define test suite driver

add_executable(lyric_packaging_testsuite ${TEST_CASES})
target_include_directories(lyric_packaging_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(lyric_packaging_testsuite PUBLIC
    lyric::lyric_packaging
    gtest::gtest
    )
gtest_discover_tests(lyric_packaging_testsuite DISCOVERY_TIMEOUT 30)

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
