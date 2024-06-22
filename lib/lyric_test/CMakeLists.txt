
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_test as a shared library
add_library(lyric_test SHARED)
add_library(lyric::lyric_test ALIAS lyric_test)

set(LYRIC_TEST_INCLUDES
    include/lyric_test/abstract_tester.h
    include/lyric_test/base_protocol_mock.h
    include/lyric_test/computation_matchers.h
    include/lyric_test/data_cell_matchers.h
    include/lyric_test/lyric_protocol_tester.h
    include/lyric_test/lyric_tester.h
    include/lyric_test/matchers.h
    include/lyric_test/mock_binder.h
    include/lyric_test/mock_receive.h
    include/lyric_test/mock_send.h
    include/lyric_test/ref_handle_matchers.h
    include/lyric_test/test_inspector.h
    include/lyric_test/test_result.h
    include/lyric_test/test_run.h
    include/lyric_test/test_run_matchers.h
    include/lyric_test/test_runner.h
    )
set_target_properties(lyric_test PROPERTIES PUBLIC_HEADER "${LYRIC_TEST_INCLUDES}")

target_sources(lyric_test PRIVATE
    src/base_protocol_mock.cpp
    src/computation_matchers.cpp
    src/data_cell_matchers.cpp
    src/lyric_protocol_tester.cpp
    src/lyric_tester.cpp
    src/mock_binder.cpp
    src/mock_receive.cpp
    src/mock_send.cpp
    src/ref_handle_matchers.cpp
    src/test_inspector.cpp
    src/test_result.cpp
    src/test_run.cpp
    src/test_run_matchers.cpp
    src/test_runner.cpp
    ${LYRIC_TEST_INCLUDES}
    )

# set the library version
set_target_properties(lyric_test PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_test PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_test PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_test PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(lyric_test
    PUBLIC
    lyric::lyric_build
    tempo::tempo_test
    tempo::tempo_utils
    gtest::gtest
    )

# install targets
install(TARGETS lyric_test EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_test
    )
