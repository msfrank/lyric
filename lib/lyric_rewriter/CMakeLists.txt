
# build lyric_rewriter as a shared library
add_library(lyric_rewriter SHARED)
add_library(lyric::lyric_rewriter ALIAS lyric_rewriter)

set(LYRIC_REWRITER_INCLUDES
    include/lyric_rewriter/abstract_macro.h
    include/lyric_rewriter/abstract_rewrite_driver.h
    include/lyric_rewriter/assembler_attrs.h
    include/lyric_rewriter/lyric_ast_base_visitor.h
    include/lyric_rewriter/lyric_ast_binary_visitor.h
    include/lyric_rewriter/lyric_ast_cond_visitor.h
    include/lyric_rewriter/lyric_ast_defclass_visitor.h
    include/lyric_rewriter/lyric_ast_defconcept_visitor.h
    include/lyric_rewriter/lyric_ast_defenum_visitor.h
    include/lyric_rewriter/lyric_ast_definstance_visitor.h
    include/lyric_rewriter/lyric_ast_defstruct_visitor.h
    include/lyric_rewriter/lyric_ast_dynamic_visitor.h
    include/lyric_rewriter/lyric_ast_for_visitor.h
    include/lyric_rewriter/lyric_ast_if_visitor.h
    include/lyric_rewriter/lyric_ast_match_visitor.h
    include/lyric_rewriter/lyric_ast_param_visitor.h
    include/lyric_rewriter/lyric_ast_sequence_visitor.h
    include/lyric_rewriter/lyric_ast_terminal_visitor.h
    include/lyric_rewriter/lyric_ast_unary_visitor.h
    include/lyric_rewriter/lyric_ast_while_visitor.h
    include/lyric_rewriter/lyric_rewriter.h
    include/lyric_rewriter/macro_block.h
    include/lyric_rewriter/macro_registry.h
    include/lyric_rewriter/macro_rewrite_driver.h
    include/lyric_rewriter/rewrite_processor.h
    include/lyric_rewriter/rewriter_result.h
    include/lyric_rewriter/schema_class_node_visitor.h
    include/lyric_rewriter/schema_vocabulary_node_visitor.h
    include/lyric_rewriter/trap_macro.h
    )
set_target_properties(lyric_rewriter PROPERTIES PUBLIC_HEADER "${LYRIC_REWRITER_INCLUDES}")

target_sources(lyric_rewriter PRIVATE
    src/assembler_attrs.cpp
    src/lyric_ast_base_visitor.cpp
    src/lyric_ast_binary_visitor.cpp
    src/lyric_ast_cond_visitor.cpp
    src/lyric_ast_defclass_visitor.cpp
    src/lyric_ast_defconcept_visitor.cpp
    src/lyric_ast_defenum_visitor.cpp
    src/lyric_ast_definstance_visitor.cpp
    src/lyric_ast_defstruct_visitor.cpp
    src/lyric_ast_dynamic_visitor.cpp
    src/lyric_ast_for_visitor.cpp
    src/lyric_ast_if_visitor.cpp
    src/lyric_ast_match_visitor.cpp
    src/lyric_ast_param_visitor.cpp
    src/lyric_ast_sequence_visitor.cpp
    src/lyric_ast_terminal_visitor.cpp
    src/lyric_ast_unary_visitor.cpp
    src/lyric_ast_while_visitor.cpp
    src/lyric_rewriter.cpp
    src/macro_block.cpp
    src/macro_registry.cpp
    src/macro_rewrite_driver.cpp
    src/rewrite_processor.cpp
    src/rewriter_result.cpp
    src/trap_macro.cpp
    )

# set the library version
set_target_properties(lyric_rewriter PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_rewriter PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_rewriter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_rewriter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(lyric_rewriter
    PUBLIC
    lyric::lyric_assembler
    lyric::lyric_importer
    lyric::lyric_parser
    lyric::lyric_runtime
    lyric::lyric_schema
    lyric::lyric_typing
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    )

# install targets
install(TARGETS lyric_rewriter EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_rewriter
    )

# add testing subdirectory
add_subdirectory(test)
