
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_serde as a shared library
add_library(lyric_serde SHARED)
add_library(lyric::lyric_serde ALIAS lyric_serde)

set(LYRIC_SERDE_INCLUDES
    include/lyric_serde/append_operation_walker.h
    include/lyric_serde/change_walker.h
    include/lyric_serde/emit_operation_walker.h
    include/lyric_serde/insert_operation_walker.h
    include/lyric_serde/lyric_patchset.h
    include/lyric_serde/lyric_serde.h
    include/lyric_serde/namespace_walker.h
    include/lyric_serde/operation_path.h
    include/lyric_serde/patchset_change.h
    include/lyric_serde/patchset_namespace.h
    include/lyric_serde/patchset_state.h
    include/lyric_serde/patchset_value.h
    include/lyric_serde/patchset_value_parser.h
    include/lyric_serde/patchset_value_writer.h
    include/lyric_serde/patchset_walker.h
    include/lyric_serde/remove_operation_walker.h
    include/lyric_serde/replace_operation_walker.h
    include/lyric_serde/serde_result.h
    include/lyric_serde/serde_types.h
    include/lyric_serde/update_operation_walker.h
    include/lyric_serde/value_walker.h
    )
set_target_properties(lyric_serde PROPERTIES PUBLIC_HEADER "${LYRIC_SERDE_INCLUDES}")

# generate flatbuffer files for patchset IDL
add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_serde/generated/patchset.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_serde/generated
    COMMAND
      ${FLATC} --cpp --scoped-enums --gen-mutable --gen-object-api --filename-suffix ''
      -o ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_serde/generated
      ${CMAKE_CURRENT_SOURCE_DIR}/share/patchset.fbs
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/patchset.fbs
)

# generate header containing the patchset schema
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_serde/generated/patchset_schema.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_serde/generated
    COMMAND
      tempo::tempo-bytes2code
      -g LYRIC_SERDE_GENERATED_PATCHSET_SCHEMA_H -n lyric_serde::schema::patchset
      ${CMAKE_CURRENT_SOURCE_DIR}/share/patchset.fbs
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_serde/generated/patchset_schema.h
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/patchset.fbs
)

target_sources(lyric_serde PRIVATE
    src/append_operation_walker.cpp
    src/change_walker.cpp
    src/emit_operation_walker.cpp
    src/insert_operation_walker.cpp
    src/lyric_patchset.cpp
    src/namespace_walker.cpp
    src/operation_path.cpp
    src/patchset_change.cpp
    src/patchset_namespace.cpp
    src/patchset_state.cpp
    src/patchset_value.cpp
    src/patchset_value_parser.cpp
    src/patchset_value_writer.cpp
    src/patchset_walker.cpp
    src/remove_operation_walker.cpp
    src/replace_operation_walker.cpp
    src/serde_result.cpp
    src/serde_types.cpp
    src/update_operation_walker.cpp
    src/value_walker.cpp

    include/lyric_serde/internal/patchset_reader.h
    src/internal/patchset_reader.cpp

    ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_serde/generated/patchset.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_serde/generated/patchset_schema.h
    )

# set the library version
set_target_properties(lyric_serde PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_serde PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_serde PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_serde PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(lyric_serde
    PUBLIC
    lyric::lyric_common
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    flatbuffers::flatbuffers
    )

# install targets
install(TARGETS lyric_serde EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_serde
    )

# install data
install(FILES share/patchset.fbs DESTINATION ${CMAKE_INSTALL_DATADIR}/lyric_serde)

# add testing subdirectory
add_subdirectory(test)
