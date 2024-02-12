
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build lyric_parser as a shared library
add_library(lyric_parser SHARED)
add_library(lyric::lyric_parser ALIAS lyric_parser)

set(LYRIC_PARSER_INCLUDES
    include/lyric_parser/archetype_attr.h
    include/lyric_parser/archetype_attr_parser.h
    include/lyric_parser/archetype_attr_writer.h
    include/lyric_parser/archetype_namespace.h
    include/lyric_parser/archetype_node.h
    include/lyric_parser/archetype_state.h
    include/lyric_parser/assignable.h
    include/lyric_parser/ast_attrs.h
    include/lyric_parser/lyric_archetype.h
    include/lyric_parser/lyric_parser.h
    include/lyric_parser/node_walker.h
    include/lyric_parser/parse_diagnostics.h
    include/lyric_parser/parse_literal.h
    include/lyric_parser/parse_result.h
    include/lyric_parser/parser_attrs.h
    include/lyric_parser/parser_types.h
)
set_target_properties(lyric_parser PROPERTIES PUBLIC_HEADER "${LYRIC_PARSER_INCLUDES}")

# generate antlr4 files for Lyric module grammar
add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleLexer.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleLexer.h
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleLexer.interp
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleLexer.tokens
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParserBaseListener.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParserBaseListener.h
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParser.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParser.h
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParser.interp
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParser.tokens
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParserListener.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParserListener.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/src
    COMMAND
      ${ANTLR}
      -o ${CMAKE_CURRENT_BINARY_DIR}/src
      -lib ${CMAKE_CURRENT_SOURCE_DIR}/share
      -Werror
      -Dlanguage=Cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/share/ModuleLexer.g4 ${CMAKE_CURRENT_SOURCE_DIR}/share/ModuleParser.g4
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/ModuleLexer.g4 ${CMAKE_CURRENT_SOURCE_DIR}/share/ModuleParser.g4
)

# generate flatbuffer files for archetype IDL
add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_parser/generated/archetype.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_parser/generated
    COMMAND
      ${FLATC} --cpp --scoped-enums --gen-mutable --gen-object-api --filename-suffix ''
      -o ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_parser/generated
      ${CMAKE_CURRENT_SOURCE_DIR}/share/archetype.fbs
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/archetype.fbs
)

# generate header containing the archetype schema
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_parser/generated/archetype_schema.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_parser/generated
    COMMAND
      tempo::tempo-bytes2code
      -g LYRIC_PARSER_GENERATED_ARCHETYPE_SCHEMA_H -n lyric_parser::schema::archetype
      ${CMAKE_CURRENT_SOURCE_DIR}/share/archetype.fbs
      ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_parser/generated/archetype_schema.h
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/archetype.fbs
)

target_sources(lyric_parser
    PRIVATE
    src/archetype_attr.cpp
    src/archetype_attr_parser.cpp
    src/archetype_attr_writer.cpp
    src/archetype_namespace.cpp
    src/archetype_node.cpp
    src/archetype_state.cpp
    src/assignable.cpp
    src/ast_attrs.cpp
    src/lyric_archetype.cpp
    src/lyric_parser.cpp
    src/node_walker.cpp
    src/parse_diagnostics.cpp
    src/parse_literal.cpp
    src/parse_result.cpp
    src/parser_attrs.cpp
    src/parser_types.cpp

    include/lyric_parser/internal/archetype_reader.h
    src/internal/archetype_reader.cpp
    include/lyric_parser/internal/lexer_error_listener.h
    src/internal/lexer_error_listener.cpp
    include/lyric_parser/internal/module_archetype.h
    src/internal/module_archetype.cpp
    include/lyric_parser/internal/module_arithmetic_ops.h
    src/internal/module_arithmetic_ops.cpp
    include/lyric_parser/internal/module_assign_ops.h
    src/internal/module_assign_ops.cpp
    include/lyric_parser/internal/module_compare_ops.h
    src/internal/module_compare_ops.cpp
    include/lyric_parser/internal/module_constant_ops.h
    src/internal/module_constant_ops.cpp
    include/lyric_parser/internal/module_construct_ops.h
    src/internal/module_construct_ops.cpp
    include/lyric_parser/internal/module_control_ops.h
    src/internal/module_control_ops.cpp
    include/lyric_parser/internal/module_defclass_ops.h
    src/internal/module_defclass_ops.cpp
    include/lyric_parser/internal/module_defconcept_ops.h
    src/internal/module_defconcept_ops.cpp
    include/lyric_parser/internal/module_defenum_ops.h
    src/internal/module_defenum_ops.cpp
    include/lyric_parser/internal/module_definstance_ops.h
    src/internal/module_definstance_ops.cpp
    include/lyric_parser/internal/module_defstruct_ops.h
    src/internal/module_defstruct_ops.cpp
    include/lyric_parser/internal/module_define_ops.h
    src/internal/module_define_ops.cpp
    include/lyric_parser/internal/module_deref_ops.h
    src/internal/module_deref_ops.cpp
    include/lyric_parser/internal/module_exception_ops.h
    src/internal/module_exception_ops.cpp
    include/lyric_parser/internal/module_logical_ops.h
    src/internal/module_logical_ops.cpp
    include/lyric_parser/internal/module_match_ops.h
    src/internal/module_match_ops.cpp
    include/lyric_parser/internal/module_parameter_ops.h
    src/internal/module_parameter_ops.cpp
    include/lyric_parser/internal/module_symbol_ops.h
    src/internal/module_symbol_ops.cpp
    include/lyric_parser/internal/parser_error_listener.h
    src/internal/parser_error_listener.cpp

    # antlr auto-generated sources
    ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleLexer.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleLexer.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParserBaseListener.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParserBaseListener.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParser.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParser.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParserListener.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/src/ModuleParserListener.h

    # flatbuffers auto-generated sources
    ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_parser/generated/archetype.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/lyric_parser/generated/archetype_schema.h
)

# set the library version
set_target_properties(lyric_parser PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(lyric_parser PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(lyric_parser PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

# set the public header include path differently on the target depending on the interface
target_include_directories(lyric_parser PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# antlr-generated files look for includes in the antlr4-runtime directory
target_include_directories(lyric_parser
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/src
    ${CONAN_INCLUDE_DIRS_ANTLRDEVEL}/antlr4-runtime
    )

target_link_libraries(lyric_parser
    PUBLIC
    lyric::lyric_common
    lyric::lyric_schema
    tempo::tempo_tracing
    tempo::tempo_utils
    absl::flat_hash_map
    PRIVATE
    antlr::antlr
    flatbuffers::flatbuffers
    icu::icu
    )

# install targets
install(TARGETS lyric_parser EXPORT lyric-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lyric_parser
    )

# install data
install(FILES share/archetype.fbs DESTINATION ${CMAKE_INSTALL_DATADIR}/lyric_parser)

# add testing subdirectory
add_subdirectory(test)
