
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_runtime as a shared library
add_library(lyric_runtime SHARED)
add_library(lyric::lyric_runtime ALIAS lyric_runtime)

set(LYRIC_RUNTIME_INCLUDES
    include/lyric_runtime/abstract_heap.h
    include/lyric_runtime/abstract_inspector.h
    include/lyric_runtime/abstract_loader.h
    include/lyric_runtime/abstract_plugin.h
    include/lyric_runtime/abstract_port_writer.h
    include/lyric_runtime/abstract_ref.h
    include/lyric_runtime/base_ref.h
    include/lyric_runtime/bytecode_interpreter.h
    include/lyric_runtime/bytecode_segment.h
    include/lyric_runtime/bytes_ref.h
    include/lyric_runtime/call_cell.h
    include/lyric_runtime/chain_loader.h
    include/lyric_runtime/data_cell.h
    include/lyric_runtime/descriptor_entry.h
    include/lyric_runtime/duplex_port.h
    include/lyric_runtime/gc_heap.h
    include/lyric_runtime/hash_data_cell.h
    include/lyric_runtime/heap_manager.h
    include/lyric_runtime/interpreter_result.h
    include/lyric_runtime/interpreter_state.h
    include/lyric_runtime/library_plugin.h
    include/lyric_runtime/literal_cell.h
    include/lyric_runtime/lyric_runtime.h
    include/lyric_runtime/native_interface.h
    include/lyric_runtime/port_multiplexer.h
    include/lyric_runtime/promise.h
    include/lyric_runtime/ref_handle.h
    include/lyric_runtime/rest_ref.h
    include/lyric_runtime/runtime_options.h
    include/lyric_runtime/runtime_types.h
    include/lyric_runtime/segment_manager.h
    include/lyric_runtime/serialize_value.h
    include/lyric_runtime/stackful_coroutine.h
    include/lyric_runtime/static_loader.h
    include/lyric_runtime/string_ref.h
    include/lyric_runtime/subroutine_manager.h
    include/lyric_runtime/system_scheduler.h
    include/lyric_runtime/task.h
    include/lyric_runtime/trap_index.h
    include/lyric_runtime/type_entry.h
    include/lyric_runtime/type_manager.h
    include/lyric_runtime/url_ref.h
    include/lyric_runtime/virtual_table.h
    )
set_target_properties(lyric_runtime PROPERTIES PUBLIC_HEADER "${LYRIC_RUNTIME_INCLUDES}")

target_sources(lyric_runtime PRIVATE
    src/base_ref.cpp
    src/bytecode_interpreter.cpp
    src/bytecode_segment.cpp
    src/bytes_ref.cpp
    src/call_cell.cpp
    src/chain_loader.cpp
    src/data_cell.cpp
    src/descriptor_entry.cpp
    src/duplex_port.cpp
    src/gc_heap.cpp
    src/heap_manager.cpp
    src/interpreter_result.cpp
    src/interpreter_state.cpp
    src/library_plugin.cpp
    src/literal_cell.cpp
    src/native_interface.cpp
    src/port_multiplexer.cpp
    src/promise.cpp
    src/ref_handle.cpp
    src/rest_ref.cpp
    src/runtime_options.cpp
    src/runtime_types.cpp
    src/segment_manager.cpp
    src/serialize_value.cpp
    src/stackful_coroutine.cpp
    src/static_loader.cpp
    src/string_ref.cpp
    src/subroutine_manager.cpp
    src/system_scheduler.cpp
    src/task.cpp
    src/trap_index.cpp
    src/type_entry.cpp
    src/type_manager.cpp
    src/url_ref.cpp
    src/virtual_table.cpp

    include/lyric_runtime/internal/get_class_virtual_table.h
    src/internal/get_class_virtual_table.cpp
    include/lyric_runtime/internal/get_concept_table.h
    src/internal/get_concept_table.cpp
    include/lyric_runtime/internal/get_enum_virtual_table.h
    src/internal/get_enum_virtual_table.cpp
    include/lyric_runtime/internal/get_existential_table.h
    src/internal/get_existential_table.cpp
    include/lyric_runtime/internal/get_instance_virtual_table.h
    src/internal/get_instance_virtual_table.cpp
    include/lyric_runtime/internal/get_struct_virtual_table.h
    src/internal/get_struct_virtual_table.cpp
    include/lyric_runtime/internal/load_utils.h
    src/internal/load_utils.cpp
    include/lyric_runtime/internal/resolve_link.h
    src/internal/resolve_link.cpp
    )

# set the library version
set_target_properties(lyric_runtime PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION "${PROJECT_VERSION_MAJOR}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_runtime PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_runtime PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

target_compile_definitions(lyric_runtime PRIVATE
    "PLUGIN_SYSTEM_NAME=\"${CMAKE_SYSTEM_NAME}\""
    "PLUGIN_SYSTEM_VERSION=\"${CMAKE_SYSTEM_VERSION}\""
    "PLUGIN_ARCHITECTURE=\"${CMAKE_SYSTEM_PROCESSOR}\""
    "PLUGIN_COMPILER_ID=\"${CMAKE_CXX_COMPILER_ID}\""
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_runtime PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(lyric_runtime
    PUBLIC
    lyric::lyric_common
    lyric::lyric_object
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    uv::uv
    PRIVATE
    absl::strings
    flatbuffers::flatbuffers
    utfcpp::utfcpp
    )

# install targets
install(TARGETS lyric_runtime EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_runtime
    )

# add testing subdirectory
add_subdirectory(test)
