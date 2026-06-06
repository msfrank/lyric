enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    connection_tests.cpp
    port_multiplexer_tests.cpp
    system_scheduler_tests.cpp
    tagged_pointer_value_tests.cpp
    tagged_value_tests.cpp
    )

# build testmod object

set(TESTMOD_OBJECT_PATH "${CMAKE_CURRENT_BINARY_DIR}/testmod.lyo")

add_executable(testmod-builder testmod_builder.cpp)
target_compile_definitions(testmod-builder PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    )
target_link_libraries(testmod-builder PUBLIC lyric::lyric_assembler)

add_custom_command (
    OUTPUT ${TESTMOD_OBJECT_PATH}
    COMMAND testmod-builder ${TESTMOD_OBJECT_PATH}
    COMMENT "generating testmod.lyo"
    DEPENDS testmod-builder
)
add_custom_target(testmod-object DEPENDS ${TESTMOD_OBJECT_PATH})

# define test suite driver

add_executable(lyric_runtime_testsuite
    ${TEST_CASES}
    base_runtime_fixture.cpp base_runtime_fixture.h
    runtime_mocks.h
    )
add_dependencies(lyric_runtime_testsuite testmod-object)
target_compile_definitions(lyric_runtime_testsuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    "TESTMOD_OBJECT_PATH=\"${TESTMOD_OBJECT_PATH}\""
    )
target_link_libraries(lyric_runtime_testsuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_runtime
    lyric::lyric_test
    tempo::tempo_test
    tempo::tempo_test_main
    gtest::gtest
    )
gtest_discover_tests(lyric_runtime_testsuite DISCOVERY_TIMEOUT 30)

# define test suite static library

add_library(LyricRuntimeTestSuite OBJECT
    ${TEST_CASES}
    base_runtime_fixture.cpp base_runtime_fixture.h
    runtime_mocks.h
    )
add_dependencies(LyricRuntimeTestSuite testmod-object)
target_compile_definitions(LyricRuntimeTestSuite PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_BUILD_BOOTSTRAP_DIR=\"${LYRIC_BUILD_BOOTSTRAP_DIR}\""
    "TESTMOD_OBJECT_PATH=\"${TESTMOD_OBJECT_PATH}\""
    )
target_link_libraries(LyricRuntimeTestSuite PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_runtime
    lyric::lyric_test
    tempo::tempo_test
    gtest::gtest
    )