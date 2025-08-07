
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_archiver as a shared library
add_library(lyric_archiver SHARED)
add_library(lyric::lyric_archiver ALIAS lyric_archiver)

set(LYRIC_COMPILER_INCLUDES
    include/lyric_archiver/archive_utils.h
    include/lyric_archiver/archiver_result.h
    include/lyric_archiver/archiver_state.h
    include/lyric_archiver/copy_action.h
    include/lyric_archiver/copy_call.h
    include/lyric_archiver/copy_class.h
    include/lyric_archiver/copy_concept.h
    include/lyric_archiver/copy_enum.h
    include/lyric_archiver/copy_field.h
    include/lyric_archiver/copy_impl.h
    include/lyric_archiver/copy_instance.h
    include/lyric_archiver/copy_proc.h
    include/lyric_archiver/copy_struct.h
    include/lyric_archiver/copy_template.h
    include/lyric_archiver/copy_type.h
    include/lyric_archiver/lyric_archiver.h
    include/lyric_archiver/scan_proc.h
    include/lyric_archiver/symbol_reference_set.h
    )
set_target_properties(lyric_archiver PROPERTIES PUBLIC_HEADER "${LYRIC_COMPILER_INCLUDES}")

target_sources(lyric_archiver PRIVATE
    src/archive_utils.cpp
    src/archiver_result.cpp
    src/archiver_state.cpp
    src/copy_action.cpp
    src/copy_call.cpp
    src/copy_class.cpp
    src/copy_concept.cpp
    src/copy_enum.cpp
    src/copy_field.cpp
    src/copy_impl.cpp
    src/copy_instance.cpp
    src/copy_proc.cpp
    src/copy_struct.cpp
    src/copy_template.cpp
    src/copy_type.cpp
    src/lyric_archiver.cpp
    src/scan_proc.cpp
    src/symbol_reference_set.cpp
    )

# set the library version
set_target_properties(lyric_archiver PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION "${PROJECT_VERSION_MAJOR}")

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
    )

# install targets
install(TARGETS lyric_archiver EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_archiver
    )

# add testing subdirectory
add_subdirectory(test)
