
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_optimizer as a shared library
add_library(lyric_optimizer SHARED
    include/lyric_optimizer/abstract_analysis.h)
add_library(lyric::lyric_optimizer ALIAS lyric_optimizer)

set(LYRIC_COMPILER_INCLUDES
    include/lyric_optimizer/abstract_directive.h
    include/lyric_optimizer/activation_state.h
    include/lyric_optimizer/basic_block.h
    include/lyric_optimizer/build_proc.h
    include/lyric_optimizer/control_flow_graph.h
    include/lyric_optimizer/data_flow_analysis.h
    include/lyric_optimizer/directive_chain.h
    include/lyric_optimizer/operand_stack.h
    include/lyric_optimizer/lyric_optimizer.h
    include/lyric_optimizer/optimizer_directives.h
    include/lyric_optimizer/optimizer_result.h
    include/lyric_optimizer/parse_proc.h
    include/lyric_optimizer/phi_function.h
    include/lyric_optimizer/value.h
    include/lyric_optimizer/variable.h
    )
set_target_properties(lyric_optimizer PROPERTIES PUBLIC_HEADER "${LYRIC_COMPILER_INCLUDES}")

target_sources(lyric_optimizer PRIVATE
    src/activation_state.cpp
    src/basic_block.cpp
    src/build_proc.cpp
    src/control_flow_graph.cpp
    src/data_flow_analysis.cpp
    src/directive_chain.cpp
    src/instance.cpp
    src/lyric_optimizer.cpp
    src/operand_stack.cpp
    src/optimizer_directives.cpp
    src/optimizer_result.cpp
    src/parse_proc.cpp
    src/phi_function.cpp
    src/value.cpp
    src/variable.cpp

    include/lyric_optimizer/internal/cfg_types.h
    include/lyric_optimizer/internal/cfg_data.h
    src/internal/cfg_data.cpp
    )

# set the library version
set_target_properties(lyric_optimizer PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_optimizer PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_optimizer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_optimizer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

target_link_libraries(lyric_optimizer
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
    Boost::headers
    )

# install targets
install(TARGETS lyric_optimizer EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_optimizer
    )

# add testing subdirectory
add_subdirectory(test)
