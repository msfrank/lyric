
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_object as a shared library
add_library(lyric_object SHARED)
add_library(lyric::lyric_object ALIAS lyric_object)

set(LYRIC_OBJECT_INCLUDES
    include/lyric_object/action_walker.h
    include/lyric_object/binding_walker.h
    include/lyric_object/bytecode_builder.h
    include/lyric_object/bytecode_iterator.h
    include/lyric_object/call_walker.h
    include/lyric_object/class_walker.h
    include/lyric_object/concept_walker.h
    include/lyric_object/concrete_type_walker.h
    include/lyric_object/enum_walker.h
    include/lyric_object/existential_walker.h
    include/lyric_object/extension_walker.h
    include/lyric_object/field_walker.h
    include/lyric_object/impl_walker.h
    include/lyric_object/import_walker.h
    include/lyric_object/instance_walker.h
    include/lyric_object/intersection_type_walker.h
    include/lyric_object/link_walker.h
    include/lyric_object/literal_walker.h
    include/lyric_object/lyric_object.h
    include/lyric_object/namespace_walker.h
    include/lyric_object/object_result.h
    include/lyric_object/object_types.h
    include/lyric_object/object_walker.h
    include/lyric_object/parameter_walker.h
    include/lyric_object/placeholder_type_walker.h
    include/lyric_object/plugin_specifier.h
    include/lyric_object/static_walker.h
    include/lyric_object/struct_walker.h
    include/lyric_object/symbol_walker.h
    include/lyric_object/template_parameter_walker.h
    include/lyric_object/template_walker.h
    include/lyric_object/type_walker.h
    include/lyric_object/union_type_walker.h
    )
set_target_properties(lyric_object PROPERTIES PUBLIC_HEADER "${LYRIC_OBJECT_INCLUDES}")

# generate flatbuffer files for object IDL
add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_object/generated/object.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_object/generated
    COMMAND
      ${FLATC} --cpp --scoped-enums --gen-mutable --gen-object-api --filename-suffix ''
      -o ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_object/generated
      ${CMAKE_CURRENT_SOURCE_DIR}/share/object.fbs
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/object.fbs
)

# generate header containing the object schema
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_object/generated/object_schema.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_object/generated
    COMMAND
      tempo::tempo-bytes2code
      -g LYRIC_OBJECT_GENERATED_OBJECT_SCHEMA_H -n lyric_object::schema::object
      ${CMAKE_CURRENT_SOURCE_DIR}/share/object.fbs
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_object/generated/object_schema.h
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/object.fbs
)

target_sources(lyric_object PRIVATE
    src/action_walker.cpp
    src/binding_walker.cpp
    src/bytecode_builder.cpp
    src/bytecode_iterator.cpp
    src/call_walker.cpp
    src/class_walker.cpp
    src/concept_walker.cpp
    src/concrete_type_walker.cpp
    src/enum_walker.cpp
    src/existential_walker.cpp
    src/extension_walker.cpp
    src/field_walker.cpp
    src/impl_walker.cpp
    src/import_walker.cpp
    src/instance_walker.cpp
    src/intersection_type_walker.cpp
    src/link_walker.cpp
    src/literal_walker.cpp
    src/lyric_object.cpp
    src/namespace_walker.cpp
    src/object_result.cpp
    src/object_types.cpp
    src/object_walker.cpp
    src/parameter_walker.cpp
    src/placeholder_type_walker.cpp
    src/plugin_specifier.cpp
    src/struct_walker.cpp
    src/static_walker.cpp
    src/symbol_walker.cpp
    src/template_parameter_walker.cpp
    src/template_walker.cpp
    src/type_walker.cpp
    src/union_type_walker.cpp

    include/lyric_object/internal/object_reader.h
    src/internal/object_reader.cpp
    include/lyric_object/internal/type_utils.h
    src/internal/type_utils.cpp

    ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_object/generated/object.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_object/generated/object_schema.h
    )

# set the library version
set_target_properties(lyric_object PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_object PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_object PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

target_compile_definitions(lyric_object PRIVATE
    "OBJECT_PLUGIN_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
    "OBJECT_PLUGIN_SYSTEM_VERSION=\"${CMAKE_SYSTEM_VERSION}\""
    "OBJECT_PLUGIN_ARCHITECTURE=\"${CMAKE_SYSTEM_PROCESSOR}\""
    "OBJECT_PLUGIN_COMPILER_ID=\"${CMAKE_CXX_COMPILER_ID}\""
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_object PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(lyric_object
    PUBLIC
    lyric::lyric_common
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    flatbuffers::flatbuffers
    icu::icu
    )

# install targets
install(TARGETS lyric_object EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_object
    )

# install data
install(FILES share/object.fbs DESTINATION ${CMAKE_INSTALL_DATADIR}/lyric_object)

# add testing subdirectory
add_subdirectory(test)
