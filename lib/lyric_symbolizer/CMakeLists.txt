
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_symbolizer as a shared library
add_library(lyric_symbolizer SHARED)
add_library(lyric::lyric_symbolizer ALIAS lyric_symbolizer)

set(LYRIC_SYMBOLIZER_INCLUDES
    include/lyric_symbolizer/lyric_symbolizer.h
    include/lyric_symbolizer/symbolizer_result.h
    include/lyric_symbolizer/symbolizer_scan_driver.h
    )
set_target_properties(lyric_symbolizer PROPERTIES PUBLIC_HEADER "${LYRIC_SYMBOLIZER_INCLUDES}")

target_sources(lyric_symbolizer PRIVATE
    src/lyric_symbolizer.cpp
    src/symbolizer_result.cpp
    src/symbolizer_scan_driver.cpp
    )

# set the library version
set_target_properties(lyric_symbolizer PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION "${PROJECT_VERSION_MAJOR}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_symbolizer PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_symbolizer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_symbolizer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(lyric_symbolizer
    PUBLIC
    lyric::lyric_assembler
    lyric::lyric_importer
    lyric::lyric_parser
    lyric::lyric_rewriter
    lyric::lyric_runtime
    lyric::lyric_typing
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    )

# install targets
install(TARGETS lyric_symbolizer EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_symbolizer
    )

# add testing subdirectory
add_subdirectory(test)
