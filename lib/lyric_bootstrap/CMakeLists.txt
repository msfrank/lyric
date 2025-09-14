
set(PRELUDE_NAME                "prelude")
set(PRELUDE_OBJECT_NAME         "${PRELUDE_NAME}.lyo")
set(PRELUDE_PLUGIN_BASENAME     "${PRELUDE_NAME}.${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(PRELUDE_PLUGIN_NAME         "${PRELUDE_PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")

set(PRELUDE_OBJECT_PATH         "${LYRIC_BUILD_BOOTSTRAP_DIR}/${PRELUDE_OBJECT_NAME}")
set(PRELUDE_PLUGIN_PATH         "${LYRIC_BUILD_BOOTSTRAP_DIR}/${PRELUDE_PLUGIN_NAME}")

add_executable(prelude-builder
    object/builder_state.cpp
    object/builder_state.h
    object/compile_any.cpp
    object/compile_any.h
    object/compile_arithmetic.cpp
    object/compile_arithmetic.h
    object/compile_binding.cpp
    object/compile_binding.h
    object/compile_bool.cpp
    object/compile_bool.h
    object/compile_bytes.cpp
    object/compile_bytes.h
    object/compile_call.cpp
    object/compile_call.h
    object/compile_category.cpp
    object/compile_category.h
    object/compile_char.cpp
    object/compile_char.h
    object/compile_class.cpp
    object/compile_class.h
    object/compile_comparison.cpp
    object/compile_comparison.h
    object/compile_concept.cpp
    object/compile_concept.h
    object/compile_intrinsic.cpp
    object/compile_intrinsic.h
    object/compile_descriptor.cpp
    object/compile_descriptor.h
    object/compile_nil.cpp
    object/compile_nil.h
    object/compile_enum.cpp
    object/compile_enum.h
    object/compile_equality.cpp
    object/compile_equality.h
    object/compile_float.cpp
    object/compile_float.h
    object/compile_function.cpp
    object/compile_function.h
    object/compile_idea.cpp
    object/compile_idea.h
    object/compile_instance.cpp
    object/compile_instance.h
    object/compile_int.cpp
    object/compile_int.h
    object/compile_iterable.cpp
    object/compile_iterable.h
    object/compile_iterator.cpp
    object/compile_iterator.h
    object/compile_map.cpp
    object/compile_map.h
    object/compile_namespace.cpp
    object/compile_namespace.h
    object/compile_undef.cpp
    object/compile_undef.h
    object/compile_object.cpp
    object/compile_object.h
    object/compile_ordered.cpp
    object/compile_ordered.h
    object/compile_pair.cpp
    object/compile_pair.h
    object/compile_prelude.cpp
    object/compile_prelude.h
    object/compile_proposition.cpp
    object/compile_proposition.h
    object/compile_record.cpp
    object/compile_record.h
    object/compile_rest.cpp
    object/compile_rest.h
    object/compile_seq.cpp
    object/compile_seq.h
    object/compile_singleton.cpp
    object/compile_singleton.h
    object/compile_status.cpp
    object/compile_status.h
    object/compile_string.cpp
    object/compile_string.h
    object/compile_struct.cpp
    object/compile_struct.h
    object/compile_tuple.cpp
    object/compile_tuple.h
    object/compile_type.cpp
    object/compile_type.h
    object/compile_unwrap.cpp
    object/compile_unwrap.h
    object/compile_url.cpp
    object/compile_url.h
    object/compile_varargs.cpp
    object/compile_varargs.h
    object/lib_builder.cpp
    include/lyric_bootstrap/internal/bootstrap_types.h
    )

target_include_directories(prelude-builder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(prelude-builder PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "PRELUDE_PLUGIN_PATH=\"${PRELUDE_PLUGIN_PATH}\""
    )

target_link_libraries(prelude-builder PRIVATE
    lyric::lyric_runtime
    tempo::tempo_utils
    flatbuffers::flatbuffers
    )

#
add_dependencies(prelude-builder prelude-plugin)

# build prelude object file
add_custom_command (
    OUTPUT ${PRELUDE_OBJECT_PATH}
    COMMAND prelude-builder ${PRELUDE_OBJECT_PATH}
    COMMENT "generating ${PRELUDE_OBJECT_NAME}"
    DEPENDS prelude-builder prelude-plugin
    )
add_custom_target(prelude-object DEPENDS ${PRELUDE_OBJECT_PATH})

# build prelude plugin as a shared library
add_library(prelude-plugin SHARED
    include/lyric_bootstrap/internal/bootstrap_types.h
    plugin/bytes_traps.cpp
    plugin/bytes_traps.h
    plugin/category_ref.cpp
    plugin/category_ref.h
    plugin/closure_ref.cpp
    plugin/closure_ref.h
    plugin/float_traps.cpp
    plugin/float_traps.h
    plugin/iterator_ref.cpp
    plugin/iterator_ref.h
    plugin/map_key.h
    plugin/map_ref.cpp
    plugin/map_ref.h
    plugin/native_prelude.cpp
    plugin/native_prelude.h
    plugin/object_ref.cpp
    plugin/object_ref.h
    plugin/pair_ref.cpp
    plugin/pair_ref.h
    plugin/rest_iterator_ref.cpp
    plugin/rest_iterator_ref.h
    plugin/rest_traps.cpp
    plugin/rest_traps.h
    plugin/record_ref.cpp
    plugin/record_ref.h
    plugin/seq_ref.cpp
    plugin/seq_ref.h
    plugin/singleton_ref.cpp
    plugin/singleton_ref.h
    plugin/status_traps.cpp
    plugin/status_traps.h
    plugin/string_traps.cpp
    plugin/string_traps.h
    plugin/url_traps.cpp
    plugin/url_traps.h
)

target_include_directories(prelude-plugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# set properties to omit the 'lib' prefix and write the plugin into the bootstrap directory
set_target_properties(prelude-plugin PROPERTIES
    LIBRARY_OUTPUT_NAME ${PRELUDE_PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_BOOTSTRAP_DIR}
    PREFIX ""
    )

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(prelude-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(prelude-plugin PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${BOOTSTRAP_RPATH}
    )

target_link_libraries(prelude-plugin PRIVATE
    lyric::lyric_runtime
    lyric::lyric_schema
    Boost::headers
    flatbuffers::flatbuffers
    )

# build lyric_bootstrap as a shared library
add_library(lyric_bootstrap SHARED)
add_library(lyric::lyric_bootstrap ALIAS lyric_bootstrap)
add_dependencies(lyric_bootstrap prelude-object prelude-plugin)

set(LYRIC_BOOTSTRAP_INCLUDES
    include/lyric_bootstrap/bootstrap_helpers.h
    include/lyric_bootstrap/bootstrap_loader.h
    include/lyric_bootstrap/bootstrap_result.h
)
set_target_properties(lyric_bootstrap PROPERTIES PUBLIC_HEADER "${LYRIC_BOOTSTRAP_INCLUDES}")

target_sources(lyric_bootstrap PRIVATE
    src/bootstrap_helpers.cpp
    src/bootstrap_loader.cpp
    src/bootstrap_result.cpp
    )

# set the library version
set_target_properties(lyric_bootstrap PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION "${PROJECT_VERSION_MAJOR}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_bootstrap PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_bootstrap PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_bootstrap PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(lyric_bootstrap PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    "LYRIC_RUNTIME_BOOTSTRAP_DIR=\"${LYRIC_RUNTIME_BOOTSTRAP_DIR}\""
    "PLUGIN_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
    "PLUGIN_ARCHITECTURE=\"${CMAKE_SYSTEM_PROCESSOR}\""
    "PLUGIN_SUFFIX=\"${CMAKE_SHARED_LIBRARY_SUFFIX}\""
)

target_link_libraries(lyric_bootstrap
    PUBLIC
    lyric::lyric_runtime
    lyric::lyric_schema
    tempo::tempo_utils
    PRIVATE
    flatbuffers::flatbuffers
)

# install object and plugin
install(FILES
    ${PRELUDE_OBJECT_PATH}
    ${PRELUDE_PLUGIN_PATH}
    DESTINATION ${BOOTSTRAP_DIR_PREFIX}
    )
install(TARGETS lyric_bootstrap EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_bootstrap
    )

# add testing subdirectory
add_subdirectory(test)
