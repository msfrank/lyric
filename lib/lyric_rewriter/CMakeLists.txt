
# build lyric_rewriter as a shared library
add_library(lyric_rewriter SHARED)
add_library(lyric::lyric_rewriter ALIAS lyric_rewriter)

set(LYRIC_REWRITER_INCLUDES
    include/lyric_rewriter/abstract_macro.h
    include/lyric_rewriter/abstract_rewrite_driver.h
    include/lyric_rewriter/ast_base_visitor.h
    include/lyric_rewriter/ast_binary_visitor.h
    include/lyric_rewriter/ast_cond_visitor.h
    include/lyric_rewriter/ast_defclass_visitor.h
    include/lyric_rewriter/ast_defconcept_visitor.h
    include/lyric_rewriter/ast_defenum_visitor.h
    include/lyric_rewriter/ast_definstance_visitor.h
    include/lyric_rewriter/ast_defstruct_visitor.h
    include/lyric_rewriter/ast_reverse_sequence_visitor.h
    include/lyric_rewriter/ast_for_visitor.h
    include/lyric_rewriter/ast_if_visitor.h
    include/lyric_rewriter/ast_match_visitor.h
    include/lyric_rewriter/ast_param_visitor.h
    include/lyric_rewriter/ast_sequence_visitor.h
    include/lyric_rewriter/ast_terminal_visitor.h
    include/lyric_rewriter/ast_unary_visitor.h
    include/lyric_rewriter/ast_while_visitor.h
    include/lyric_rewriter/fail_unknown_visitor.h
    include/lyric_rewriter/lyric_rewriter.h
    include/lyric_rewriter/macro_block.h
    include/lyric_rewriter/macro_registry.h
    include/lyric_rewriter/macro_rewrite_driver.h
    include/lyric_rewriter/pragma_context.h
    include/lyric_rewriter/pragma_rewriter.h
    include/lyric_rewriter/rewrite_processor.h
    include/lyric_rewriter/rewriter_result.h
    include/lyric_rewriter/schema_class_node_visitor.h
    include/lyric_rewriter/schema_vocabulary_node_visitor.h
    include/lyric_rewriter/skip_unknown_visitor.h
    include/lyric_rewriter/visitor_registry.h
    )
set_target_properties(lyric_rewriter PROPERTIES PUBLIC_HEADER "${LYRIC_REWRITER_INCLUDES}")

target_sources(lyric_rewriter PRIVATE
    src/ast_base_visitor.cpp
    src/ast_binary_visitor.cpp
    src/ast_cond_visitor.cpp
    src/ast_defclass_visitor.cpp
    src/ast_defconcept_visitor.cpp
    src/ast_defenum_visitor.cpp
    src/ast_definstance_visitor.cpp
    src/ast_defstruct_visitor.cpp
    src/ast_reverse_sequence_visitor.cpp
    src/ast_for_visitor.cpp
    src/ast_if_visitor.cpp
    src/ast_match_visitor.cpp
    src/ast_param_visitor.cpp
    src/ast_sequence_visitor.cpp
    src/ast_terminal_visitor.cpp
    src/ast_unary_visitor.cpp
    src/ast_while_visitor.cpp
    src/fail_unknown_visitor.cpp
    src/lyric_rewriter.cpp
    src/macro_block.cpp
    src/macro_registry.cpp
    src/macro_rewrite_driver.cpp
    src/pragma_context.cpp
    src/pragma_rewriter.cpp
    src/rewrite_processor.cpp
    src/rewriter_result.cpp
    src/skip_unknown_visitor.cpp
    src/visitor_registry.cpp
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
    lyric::lyric_parser
    lyric::lyric_runtime
    lyric::lyric_schema
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
