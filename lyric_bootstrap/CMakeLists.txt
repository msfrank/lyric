
set(PRELUDE_NAME                "prelude")
set(PRELUDE_ASSEMBLY_NAME       "${PRELUDE_NAME}.lyo")
set(PRELUDE_PLUGIN_BASENAME     "${PRELUDE_NAME}.${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(PRELUDE_PLUGIN_NAME         "${PRELUDE_PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")

set(PRELUDE_ASSEMBLY_PATH       "${LYRIC_BUILD_BOOTSTRAP_DIR}/${PRELUDE_ASSEMBLY_NAME}")
set(PRELUDE_PLUGIN_PATH         "${LYRIC_BUILD_BOOTSTRAP_DIR}/${PRELUDE_PLUGIN_NAME}")

add_executable(prelude-builder
    assembly/builder_state.cpp
    assembly/builder_state.h
    assembly/compile_any.cpp
    assembly/compile_any.h
    assembly/compile_arithmetic.cpp
    assembly/compile_arithmetic.h
    assembly/compile_bool.cpp
    assembly/compile_bool.h
    assembly/compile_call.cpp
    assembly/compile_call.h
    assembly/compile_category.cpp
    assembly/compile_category.h
    assembly/compile_char.cpp
    assembly/compile_char.h
    assembly/compile_class.cpp
    assembly/compile_class.h
    assembly/compile_closure.cpp
    assembly/compile_closure.h
    assembly/compile_code.cpp
    assembly/compile_code.h
    assembly/compile_comparison.cpp
    assembly/compile_comparison.h
    assembly/compile_concept.cpp
    assembly/compile_concept.h
    assembly/compile_intrinsic.cpp
    assembly/compile_intrinsic.h
    assembly/compile_descriptor.cpp
    assembly/compile_descriptor.h
    assembly/compile_nil.cpp
    assembly/compile_nil.h
    assembly/compile_enum.cpp
    assembly/compile_enum.h
    assembly/compile_equality.cpp
    assembly/compile_equality.h
    assembly/compile_float.cpp
    assembly/compile_float.h
    assembly/compile_function.cpp
    assembly/compile_function.h
    assembly/compile_idea.cpp
    assembly/compile_idea.h
    assembly/compile_instance.cpp
    assembly/compile_instance.h
    assembly/compile_int.cpp
    assembly/compile_int.h
    assembly/compile_iterable.cpp
    assembly/compile_iterable.h
    assembly/compile_iterator.cpp
    assembly/compile_iterator.h
    assembly/compile_map.cpp
    assembly/compile_map.h
    assembly/compile_namespace.cpp
    assembly/compile_namespace.h
    assembly/compile_undef.cpp
    assembly/compile_undef.h
    assembly/compile_object.cpp
    assembly/compile_object.h
    assembly/compile_ordered.cpp
    assembly/compile_ordered.h
    assembly/compile_pair.cpp
    assembly/compile_pair.h
    assembly/compile_prelude.cpp
    assembly/compile_prelude.h
    assembly/compile_proposition.cpp
    assembly/compile_proposition.h
    assembly/compile_record.cpp
    assembly/compile_record.h
    assembly/compile_rest.cpp
    assembly/compile_rest.h
    assembly/compile_seq.cpp
    assembly/compile_seq.h
    assembly/compile_singleton.cpp
    assembly/compile_singleton.h
    assembly/compile_status.cpp
    assembly/compile_status.h
    assembly/compile_string.cpp
    assembly/compile_string.h
    assembly/compile_struct.cpp
    assembly/compile_struct.h
    assembly/compile_tuple.cpp
    assembly/compile_tuple.h
    assembly/compile_unwrap.cpp
    assembly/compile_unwrap.h
    assembly/compile_url.cpp
    assembly/compile_url.h
    assembly/compile_utf8.cpp
    assembly/compile_utf8.h
    assembly/compile_varargs.cpp
    assembly/compile_varargs.h
    assembly/lib_builder.cpp
    include/lyric_bootstrap/internal/bootstrap_types.h
    )

target_include_directories(prelude-builder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(prelude-builder PRIVATE
    lyric::lyric_runtime
    tempo::tempo_utils
    flatbuffers::flatbuffers
    )

# build prelude assembly file
add_custom_command (
    OUTPUT ${PRELUDE_ASSEMBLY_PATH}
    COMMAND prelude-builder ${PRELUDE_ASSEMBLY_PATH}
    COMMENT "generating ${PRELUDE_ASSEMBLY_NAME}"
    DEPENDS prelude-builder
)
add_custom_target(prelude-assembly DEPENDS ${PRELUDE_ASSEMBLY_PATH})

# build prelude plugin as a shared library
add_library(prelude-plugin SHARED
    include/lyric_bootstrap/internal/bootstrap_types.h
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
    plugin/record_ref.cpp
    plugin/record_ref.h
    plugin/seq_ref.cpp
    plugin/seq_ref.h
    plugin/singleton_ref.cpp
    plugin/singleton_ref.h
    plugin/status_ref.cpp
    plugin/status_ref.h
    plugin/string_ref.cpp
    plugin/string_ref.h
    plugin/url_ref.cpp
    plugin/url_ref.h
    plugin/utf8_traps.cpp
    plugin/utf8_traps.h
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

target_link_libraries(prelude-plugin PRIVATE
    lyric::lyric_runtime
    lyric::lyric_schema
    lyric::lyric_serde
    Boost::headers
    flatbuffers::flatbuffers
    )

# build lyric_bootstrap as a shared library
add_library(lyric_bootstrap SHARED)
add_library(lyric::lyric_bootstrap ALIAS lyric_bootstrap)
add_dependencies(lyric_bootstrap prelude-assembly prelude-plugin)

set(LYRIC_BOOTSTRAP_INCLUDES
    include/lyric_bootstrap/bootstrap_loader.h
    include/lyric_bootstrap/bootstrap_result.h
    include/lyric_bootstrap/bootstrap_types.h
    )
set_target_properties(lyric_bootstrap PROPERTIES PUBLIC_HEADER "${LYRIC_BOOTSTRAP_INCLUDES}")

target_sources(lyric_bootstrap PRIVATE
    src/bootstrap_loader.cpp
    src/bootstrap_result.cpp
    )

# set the library version
set_target_properties(lyric_bootstrap PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_bootstrap PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_bootstrap PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_bootstrap PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(lyric_bootstrap PRIVATE
    "LYRIC_RUNTIME_BOOTSTRAP_DIR=\"${LYRIC_RUNTIME_BOOTSTRAP_DIR}\""
    "ASSEMBLY_PLUGIN_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
    "ASSEMBLY_PLUGIN_ARCHITECTURE=\"${CMAKE_SYSTEM_PROCESSOR}\""
    "ASSEMBLY_PLUGIN_SUFFIX=\"${CMAKE_SHARED_LIBRARY_SUFFIX}\""
)

target_link_libraries(lyric_bootstrap
    PUBLIC
    lyric::lyric_runtime
    lyric::lyric_schema
    tempo::tempo_utils
    PRIVATE
    flatbuffers::flatbuffers
)

# install assembly and plugin
install(FILES
    ${PRELUDE_ASSEMBLY_PATH}
    ${PRELUDE_PLUGIN_PATH}
    DESTINATION ${BOOTSTRAP_DIR_PREFIX}
    )
install(TARGETS lyric_bootstrap EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_bootstrap
    )

# add testing subdirectory
add_subdirectory(test)
