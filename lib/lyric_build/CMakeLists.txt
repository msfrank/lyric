
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_build as a shared library
add_library(lyric_build SHARED)
add_library(lyric::lyric_build ALIAS lyric_build)

# generate flatbuffer files for metadata IDL
add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_build/generated/metadata.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_build/generated
    COMMAND
      ${FLATC} --cpp --scoped-enums --gen-mutable --gen-object-api --filename-suffix ''
      -o ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_build/generated
      ${CMAKE_CURRENT_SOURCE_DIR}/share/metadata.fbs
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/share/metadata.fbs
)

# generate header containing the metadata schema
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_build/generated/metadata_schema.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_build/generated
    COMMAND
      tempo::tempo-bytes2code
      -g LYRIC_SERDE_GENERATED_PATCHSET_SCHEMA_H -n lyric_build::schema::metadata
      ${CMAKE_CURRENT_SOURCE_DIR}/share/metadata.fbs
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_build/generated/metadata_schema.h
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/metadata.fbs
)

set(LYRIC_BUILD_INCLUDES
    include/lyric_build/abstract_cache.h
    include/lyric_build/abstract_filesystem.h
    include/lyric_build/artifact_loader.h
    include/lyric_build/base_task.h
    include/lyric_build/build_attrs.h
    include/lyric_build/build_conversions.h
    include/lyric_build/build_diagnostics.h
    include/lyric_build/build_result.h
    include/lyric_build/build_runner.h
    include/lyric_build/build_state.h
    include/lyric_build/build_types.h
    include/lyric_build/config_store.h
    include/lyric_build/dependency_loader.h
    include/lyric_build/local_filesystem.h
    include/lyric_build/lyric_build.h
    include/lyric_build/lyric_builder.h
    include/lyric_build/lyric_metadata.h
    include/lyric_build/memory_cache.h
    include/lyric_build/metadata_attr.h
    include/lyric_build/metadata_attr_parser.h
    include/lyric_build/metadata_attr_writer.h
    include/lyric_build/metadata_matcher.h
    include/lyric_build/metadata_namespace.h
    include/lyric_build/metadata_result.h
    include/lyric_build/metadata_state.h
    include/lyric_build/metadata_walker.h
    include/lyric_build/metadata_writer.h
    include/lyric_build/rocksdb_cache.h
    include/lyric_build/script_environment.h
    include/lyric_build/sha256_hash.h
    include/lyric_build/symbol_map.h
    include/lyric_build/target_computation.h
    include/lyric_build/task_hasher.h
    include/lyric_build/task_notification.h
    include/lyric_build/task_registry.h
    )
set_target_properties(lyric_build PROPERTIES PUBLIC_HEADER "${LYRIC_BUILD_INCLUDES}")

target_sources(lyric_build PRIVATE
    src/artifact_loader.cpp
    src/base_task.cpp
    src/build_attrs.cpp
    src/build_conversions.cpp
    src/build_diagnostics.cpp
    src/build_result.cpp
    src/build_runner.cpp
    src/build_state.cpp
    src/build_types.cpp
    src/config_store.cpp
    src/dependency_loader.cpp
    src/local_filesystem.cpp
    src/lyric_builder.cpp
    src/lyric_metadata.cpp
    src/memory_cache.cpp
    src/metadata_attr.cpp
    src/metadata_attr_parser.cpp
    src/metadata_attr_writer.cpp
    src/metadata_matcher.cpp
    src/metadata_namespace.cpp
    src/metadata_result.cpp
    src/metadata_state.cpp
    src/metadata_walker.cpp
    src/metadata_writer.cpp
    src/rocksdb_cache.cpp
    src/script_environment.cpp
    src/sha256_hash.cpp
    src/symbol_map.cpp
    src/target_computation.cpp
    src/task_hasher.cpp
    src/task_notification.cpp
    src/task_registry.cpp

    include/lyric_build/internal/analyze_module_task.h
    src/internal/analyze_module_task.cpp
    include/lyric_build/internal/archive_task.h
    src/internal/archive_task.cpp
    include/lyric_build/internal/build_task.h
    src/internal/build_task.cpp
    include/lyric_build/internal/compile_module_task.h
    src/internal/compile_module_task.cpp
    include/lyric_build/internal/compile_script_task.h
    src/internal/compile_script_task.cpp
    include/lyric_build/internal/compile_task.h
    src/internal/compile_task.cpp
    include/lyric_build/internal/metadata_reader.h
    src/internal/metadata_reader.cpp
    include/lyric_build/internal/orchestrate_task.h
    src/internal/orchestrate_task.cpp
    include/lyric_build/internal/package_task.h
    src/internal/package_task.cpp
    include/lyric_build/internal/parse_module_task.h
    src/internal/parse_module_task.cpp
    include/lyric_build/internal/symbolize_module_task.h
    src/internal/symbolize_module_task.cpp
    include/lyric_build/internal/task_utils.h
    src/internal/task_utils.cpp
    include/lyric_build/internal/test_task.h
    src/internal/test_task.cpp

    ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_build/generated/metadata.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_build/generated/metadata_schema.h
    )

# set the library version
set_target_properties(lyric_build PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_build PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_build PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

target_compile_definitions(lyric_build PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
)

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_build PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(lyric_build
    PUBLIC
    lyric::lyric_analyzer
    lyric::lyric_bootstrap
    lyric::lyric_compiler
    lyric::lyric_packaging
    lyric::lyric_parser
    lyric::lyric_runtime
    lyric::lyric_symbolizer
    tempo::tempo_tracing
    tempo::tempo_utils
    rocksdb::rocksdb
    PRIVATE
    flatbuffers::flatbuffers
    openssl::openssl
    uv::uv
    )

# install targets
install(TARGETS lyric_build EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_build
    )

# install data
install(FILES share/metadata.fbs DESTINATION ${CMAKE_INSTALL_DATADIR}/lyric_build)

# add testing subdirectory
add_subdirectory(test)
