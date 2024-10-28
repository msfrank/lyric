
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_assembler as a shared library
add_library(lyric_assembler SHARED)
add_library(lyric::lyric_assembler ALIAS lyric_assembler)

set(LYRIC_ASSEMBLER_INCLUDES
    include/lyric_assembler/abstract_callable.h
    include/lyric_assembler/abstract_callsite_reifier.h
    include/lyric_assembler/abstract_constructable.h
    include/lyric_assembler/abstract_instruction.h
    include/lyric_assembler/abstract_member_reifier.h
    include/lyric_assembler/abstract_placement.h
    include/lyric_assembler/abstract_resolver.h
    include/lyric_assembler/abstract_symbol.h
    include/lyric_assembler/action_callable.h
    include/lyric_assembler/action_symbol.h
    include/lyric_assembler/argument_variable.h
    include/lyric_assembler/assembler_attrs.h
    include/lyric_assembler/assembler_instructions.h
    include/lyric_assembler/assembler_result.h
    include/lyric_assembler/assembler_tracer.h
    include/lyric_assembler/assembler_types.h
    include/lyric_assembler/base_handle.h
    include/lyric_assembler/base_symbol.h
    include/lyric_assembler/block_handle.h
    include/lyric_assembler/callable_invoker.h
    include/lyric_assembler/call_symbol.h
    include/lyric_assembler/class_symbol.h
    include/lyric_assembler/code_fragment.h
    include/lyric_assembler/concept_symbol.h
    include/lyric_assembler/constructable_invoker.h
    include/lyric_assembler/ctor_constructable.h
    include/lyric_assembler/enum_symbol.h
    include/lyric_assembler/existential_callable.h
    include/lyric_assembler/existential_symbol.h
    include/lyric_assembler/extension_callable.h
    include/lyric_assembler/field_symbol.h
    include/lyric_assembler/fundamental_cache.h
    include/lyric_assembler/function_callable.h
    include/lyric_assembler/impl_cache.h
    include/lyric_assembler/impl_handle.h
    include/lyric_assembler/import_cache.h
    include/lyric_assembler/instance_symbol.h
    include/lyric_assembler/lexical_variable.h
    include/lyric_assembler/literal_cache.h
    include/lyric_assembler/literal_handle.h
    include/lyric_assembler/local_variable.h
    include/lyric_assembler/method_callable.h
    include/lyric_assembler/namespace_symbol.h
    include/lyric_assembler/object_root.h
    include/lyric_assembler/object_state.h
    include/lyric_assembler/object_writer.h
    include/lyric_assembler/pack_builder.h
    include/lyric_assembler/proc_builder.h
    include/lyric_assembler/proc_handle.h
    include/lyric_assembler/static_symbol.h
    include/lyric_assembler/struct_symbol.h
    include/lyric_assembler/symbol_cache.h
    include/lyric_assembler/synthetic_symbol.h
    include/lyric_assembler/template_handle.h
    include/lyric_assembler/type_cache.h
    include/lyric_assembler/type_handle.h
    include/lyric_assembler/type_set.h
    include/lyric_assembler/undeclared_symbol.h
    )
set_target_properties(lyric_assembler PROPERTIES PUBLIC_HEADER "${LYRIC_ASSEMBLER_INCLUDES}")

target_sources(lyric_assembler PRIVATE
    src/action_callable.cpp
    src/action_symbol.cpp
    src/argument_variable.cpp
    src/assembler_attrs.cpp
    src/assembler_instructions.cpp
    src/assembler_result.cpp
    src/assembler_tracer.cpp
    src/assembler_types.cpp
    src/block_handle.cpp
    src/callable_invoker.cpp
    src/call_symbol.cpp
    src/class_symbol.cpp
    src/code_fragment.cpp
    src/concept_symbol.cpp
    src/constructable_invoker.cpp
    src/ctor_constructable.cpp
    src/enum_symbol.cpp
    src/existential_callable.cpp
    src/existential_symbol.cpp
    src/extension_callable.cpp
    src/field_symbol.cpp
    src/function_callable.cpp
    src/fundamental_cache.cpp
    src/impl_cache.cpp
    src/impl_handle.cpp
    src/import_cache.cpp
    src/instance_symbol.cpp
    src/lexical_variable.cpp
    src/literal_cache.cpp
    src/literal_handle.cpp
    src/local_variable.cpp
    src/method_callable.cpp
    src/namespace_symbol.cpp
    src/object_root.cpp
    src/object_state.cpp
    src/object_writer.cpp
    src/pack_builder.cpp
    src/proc_builder.cpp
    src/proc_handle.cpp
    src/static_symbol.cpp
    src/struct_symbol.cpp
    src/symbol_cache.cpp
    src/synthetic_symbol.cpp
    src/template_handle.cpp
    src/type_cache.cpp
    src/type_handle.cpp
    src/type_set.cpp
    src/undeclared_symbol.cpp

    include/lyric_assembler/internal/call_inline.h
    src/internal/call_inline.cpp
    include/lyric_assembler/internal/import_proc.h
    src/internal/import_proc.cpp
    include/lyric_assembler/internal/writer_utils.h
    src/internal/writer_utils.cpp
    include/lyric_assembler/internal/write_actions.h
    src/internal/write_actions.cpp
    include/lyric_assembler/internal/write_calls.h
    src/internal/write_calls.cpp
    include/lyric_assembler/internal/write_classes.h
    src/internal/write_classes.cpp
    include/lyric_assembler/internal/write_concepts.h
    src/internal/write_concepts.cpp
    include/lyric_assembler/internal/write_enums.h
    src/internal/write_enums.cpp
    include/lyric_assembler/internal/write_existentials.h
    src/internal/write_existentials.cpp
    include/lyric_assembler/internal/write_fields.h
    src/internal/write_fields.cpp
    include/lyric_assembler/internal/write_impls.h
    src/internal/write_impls.cpp
    include/lyric_assembler/internal/write_imports.h
    src/internal/write_imports.cpp
    include/lyric_assembler/internal/write_instances.h
    src/internal/write_instances.cpp
    include/lyric_assembler/internal/write_links.h
    src/internal/write_links.cpp
    include/lyric_assembler/internal/write_literals.h
    src/internal/write_literals.cpp
    include/lyric_assembler/internal/write_namespaces.h
    src/internal/write_namespaces.cpp
    include/lyric_assembler/internal/write_object.h
    src/internal/write_object.cpp
    include/lyric_assembler/internal/write_statics.h
    src/internal/write_statics.cpp
    include/lyric_assembler/internal/write_structs.h
    src/internal/write_structs.cpp
    include/lyric_assembler/internal/write_templates.h
    src/internal/write_templates.cpp
    include/lyric_assembler/internal/write_types.h
    src/internal/write_types.cpp
    )

# set the library version
set_target_properties(lyric_assembler PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_assembler PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_assembler PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

target_compile_definitions(lyric_assembler PRIVATE
    "BOOTSTRAP_PRELUDE_LOCATION=\"${BOOTSTRAP_PRELUDE_LOCATION}\""
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_assembler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(lyric_assembler
    PUBLIC
    lyric::lyric_bootstrap
    lyric::lyric_importer
    lyric::lyric_runtime
    tempo::tempo_tracing
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    flatbuffers::flatbuffers
    icu::icu
    )

# install targets
install(TARGETS lyric_assembler EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_assembler
    )

# add testing subdirectory
add_subdirectory(test)
