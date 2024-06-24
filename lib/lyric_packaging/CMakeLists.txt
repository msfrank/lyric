
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_packaging as a shared library
add_library(lyric_packaging SHARED)
add_library(lyric::lyric_packaging ALIAS lyric_packaging)

set(LYRIC_PACKAGING_INCLUDES
    include/lyric_packaging/directory_loader.h
    include/lyric_packaging/entry_path.h
    include/lyric_packaging/entry_walker.h
    include/lyric_packaging/lyric_manifest.h
    include/lyric_packaging/manifest_attr.h
    include/lyric_packaging/manifest_attr_parser.h
    include/lyric_packaging/manifest_attr_writer.h
    include/lyric_packaging/manifest_entry.h
    include/lyric_packaging/manifest_namespace.h
    include/lyric_packaging/manifest_state.h
    include/lyric_packaging/manifest_walker.h
    include/lyric_packaging/package_attrs.h
    include/lyric_packaging/package_loader.h
    include/lyric_packaging/package_reader.h
    include/lyric_packaging/package_result.h
    include/lyric_packaging/package_specifier.h
    include/lyric_packaging/package_types.h
    include/lyric_packaging/package_writer.h
    )
set_target_properties(lyric_packaging PROPERTIES PUBLIC_HEADER "${LYRIC_PACKAGING_INCLUDES}")

# generate flatbuffer files for manifest IDL
add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_packaging/generated/manifest.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_packaging/generated
    COMMAND
      ${FLATC} --cpp --scoped-enums --gen-mutable --gen-object-api --filename-suffix ''
      -o ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_packaging/generated
      ${CMAKE_CURRENT_SOURCE_DIR}/share/manifest.fbs
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/manifest.fbs
)

# generate header containing the manifest schema
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_packaging/generated/manifest_schema.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_packaging/generated
    COMMAND
      tempo::tempo-bytes2code
      -g LYRIC_PACKAGING_GENERATED_MANIFEST_SCHEMA_H -n lyric_packaging::schema::manifest
      ${CMAKE_CURRENT_SOURCE_DIR}/share/manifest.fbs
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_packaging/generated/manifest_schema.h
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/manifest.fbs
)

target_sources(lyric_packaging PRIVATE
    src/directory_loader.cpp
    src/entry_path.cpp
    src/entry_walker.cpp
    src/lyric_manifest.cpp
    src/manifest_attr.cpp
    src/manifest_attr_parser.cpp
    src/manifest_attr_writer.cpp
    src/manifest_entry.cpp
    src/manifest_namespace.cpp
    src/manifest_state.cpp
    src/manifest_walker.cpp
    src/package_attrs.cpp
    src/package_loader.cpp
    src/package_reader.cpp
    src/package_result.cpp
    src/package_specifier.cpp
    src/package_types.cpp
    src/package_writer.cpp

    include/lyric_packaging/internal/manifest_reader.h
    src/internal/manifest_reader.cpp

    ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_packaging/generated/manifest.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_packaging/generated/manifest_schema.h
    )

# set the library version
set_target_properties(lyric_packaging PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_packaging PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_packaging PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_packaging PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(lyric_packaging PRIVATE
    "ASSEMBLY_PLUGIN_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
    "ASSEMBLY_PLUGIN_ARCHITECTURE=\"${CMAKE_SYSTEM_PROCESSOR}\""
    "ASSEMBLY_PLUGIN_SUFFIX=\"${CMAKE_SHARED_LIBRARY_SUFFIX}\""
    )

# make private headers visible internally
target_include_directories(lyric_packaging PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src)

target_link_libraries(lyric_packaging
    PUBLIC
    lyric::lyric_runtime
    lyric::lyric_schema
    tempo::tempo_utils
    PRIVATE
    flatbuffers::flatbuffers
    )

# install targets
install(TARGETS lyric_packaging EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_packaging
    )

# install data
install(FILES share/manifest.fbs DESTINATION ${CMAKE_INSTALL_DATADIR}/lyric_packaging)

# add testing subdirectory
add_subdirectory(test)
