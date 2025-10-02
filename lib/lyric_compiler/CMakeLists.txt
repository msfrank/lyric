
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_compiler as a shared library
add_library(lyric_compiler SHARED)
add_library(lyric::lyric_compiler ALIAS lyric_compiler)

set(LYRIC_COMPILER_INCLUDES
    include/lyric_compiler/abstract_behavior.h
    include/lyric_compiler/action_handler.h
    include/lyric_compiler/assembler_handler.h
    include/lyric_compiler/assignment_handler.h
    include/lyric_compiler/base_choice.h
    include/lyric_compiler/base_grouping.h
    include/lyric_compiler/base_invokable_handler.h
    include/lyric_compiler/binary_operation_handler.h
    include/lyric_compiler/block_handler.h
    include/lyric_compiler/compiler_handler.h
    include/lyric_compiler/compiler_macro_visitor.h
    include/lyric_compiler/compiler_result.h
    include/lyric_compiler/compiler_scan_driver.h
    include/lyric_compiler/compiler_utils.h
    include/lyric_compiler/conditional_handler.h
    include/lyric_compiler/constant_utils.h
    include/lyric_compiler/constructor_handler.h
    include/lyric_compiler/def_handler.h
    include/lyric_compiler/defalias_handler.h
    include/lyric_compiler/defclass_handler.h
    include/lyric_compiler/defclass_utils.h
    include/lyric_compiler/defconcept_handler.h
    include/lyric_compiler/defconcept_utils.h
    include/lyric_compiler/defenum_handler.h
    include/lyric_compiler/defenum_utils.h
    include/lyric_compiler/definstance_handler.h
    include/lyric_compiler/definstance_utils.h
    include/lyric_compiler/defstatic_handler.h
    include/lyric_compiler/defstruct_handler.h
    include/lyric_compiler/defstruct_utils.h
    include/lyric_compiler/data_deref_handler.h
    include/lyric_compiler/deref_utils.h
    include/lyric_compiler/entry_handler.h
    include/lyric_compiler/expect_handler.h
    include/lyric_compiler/impl_handler.h
    include/lyric_compiler/impl_utils.h
    include/lyric_compiler/import_handler.h
    include/lyric_compiler/iteration_handler.h
    include/lyric_compiler/lambda_handler.h
    include/lyric_compiler/lambda_utils.h
    include/lyric_compiler/lyric_compiler.h
    include/lyric_compiler/match_handler.h
    include/lyric_compiler/match_utils.h
    include/lyric_compiler/member_handler.h
    include/lyric_compiler/method_handler.h
    include/lyric_compiler/namespace_handler.h
    include/lyric_compiler/new_handler.h
    include/lyric_compiler/operator_utils.h
    include/lyric_compiler/form_handler.h
    include/lyric_compiler/pack_handler.h
    include/lyric_compiler/proc_handler.h
    include/lyric_compiler/resolve_utils.h
    include/lyric_compiler/symbol_deref_handler.h
    include/lyric_compiler/target_handler.h
    include/lyric_compiler/try_handler.h
    include/lyric_compiler/typename_handler.h
    include/lyric_compiler/type_utils.h
    include/lyric_compiler/unary_operation_handler.h
    include/lyric_compiler/unpack_handler.h
    include/lyric_compiler/using_handler.h
    include/lyric_compiler/variable_handler.h
    include/lyric_compiler/visitor_context.h
    )
set_target_properties(lyric_compiler PROPERTIES PUBLIC_HEADER "${LYRIC_COMPILER_INCLUDES}")

target_sources(lyric_compiler PRIVATE
    src/action_handler.cpp
    src/assembler_handler.cpp
    src/assignment_handler.cpp
    src/base_choice.cpp
    src/base_grouping.cpp
    src/base_invokable_handler.cpp
    src/binary_operation_handler.cpp
    src/block_handler.cpp
    src/compiler_handler.cpp
    src/compiler_macro_visitor.cpp
    src/compiler_result.cpp
    src/compiler_scan_driver.cpp
    src/compiler_utils.cpp
    src/conditional_handler.cpp
    src/constant_utils.cpp
    src/constructor_handler.cpp
    src/def_handler.cpp
    src/defalias_handler.cpp
    src/defclass_handler.cpp
    src/defclass_utils.cpp
    src/defconcept_handler.cpp
    src/defconcept_utils.cpp
    src/defenum_handler.cpp
    src/defenum_utils.cpp
    src/definstance_handler.cpp
    src/definstance_utils.cpp
    src/defstatic_handler.cpp
    src/defstruct_handler.cpp
    src/defstruct_utils.cpp
    src/data_deref_handler.cpp
    src/deref_utils.cpp
    src/entry_handler.cpp
    src/expect_handler.cpp
    src/impl_handler.cpp
    src/impl_utils.cpp
    src/import_handler.cpp
    src/iteration_handler.cpp
    src/lambda_handler.cpp
    src/lambda_utils.cpp
    src/lyric_compiler.cpp
    src/match_handler.cpp
    src/match_utils.cpp
    src/member_handler.cpp
    src/method_handler.cpp
    src/namespace_handler.cpp
    src/new_handler.cpp
    src/operator_utils.cpp
    src/form_handler.cpp
    src/pack_handler.cpp
    src/proc_handler.cpp
    src/resolve_utils.cpp
    src/symbol_deref_handler.cpp
    src/target_handler.cpp
    src/try_handler.cpp
    src/typename_handler.cpp
    src/type_utils.cpp
    src/unary_operation_handler.cpp
    src/unpack_handler.cpp
    src/using_handler.cpp
    src/variable_handler.cpp
    src/visitor_context.cpp

    include/lyric_compiler/internal/push_result_macro.h
    src/internal/push_result_macro.cpp
    )

# set the library version
set_target_properties(lyric_compiler PROPERTIES VERSION "${PROJECT_VERSION}" SOVERSION "${PROJECT_VERSION_MAJOR}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_compiler PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_compiler PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LYRIC_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_compiler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(lyric_compiler
    PUBLIC
    lyric::lyric_assembler
    lyric::lyric_importer
    lyric::lyric_parser
    lyric::lyric_object
    lyric::lyric_rewriter
    lyric::lyric_typing
    tempo::tempo_tracing
    tempo::tempo_utils
    absl::flat_hash_map
    absl::flat_hash_set
    PRIVATE
    absl::strings
    )

# install targets
install(TARGETS lyric_compiler EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_compiler
    )

# add testing subdirectory
add_subdirectory(test)
