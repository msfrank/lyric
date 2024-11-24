
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_archiver as a shared library
add_library(lyric_archiver SHARED)
add_library(lyric::lyric_archiver ALIAS lyric_archiver)

set(LYRIC_COMPILER_INCLUDES
    include/lyric_archiver/archiver_result.h
    include/lyric_archiver/archiver_state.h
    include/lyric_archiver/copy_call.h
    include/lyric_archiver/copy_proc.h
    include/lyric_archiver/copy_template.h
    include/lyric_archiver/lyric_archiver.h
    )
set_target_properties(lyric_archiver PROPERTIES PUBLIC_HEADER "${LYRIC_COMPILER_INCLUDES}")

target_sources(lyric_archiver PRIVATE
    src/archiver_result.cpp
    src/archiver_state.cpp
    src/copy_call.cpp
    src/copy_proc.cpp
    src/copy_template.cpp
    src/lyric_archiver.cpp
    )

# set the library version
set_target_properties(lyric_archiver PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_archiver PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_archiver PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_archiver PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(lyric_archiver
    PUBLIC
    lyric::lyric_assembler
    lyric::lyric_importer
    lyric::lyric_object
    lyric::lyric_typing
    tempo::tempo_security
    tempo::tempo_tracing
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    icu::icu
    )

# install targets
install(TARGETS lyric_archiver EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_archiver
    )

# add testing subdirectory
add_subdirectory(test)
