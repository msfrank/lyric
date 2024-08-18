
#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compiler_utils.h>
#include <lyric_compiler/internal/compile_block.h>
#include <lyric_compiler/internal/compile_namespace.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

/**
 *
 * @param block
 * @param walker
 * @return
 */
tempo_utils::Status
lyric_compiler::internal::compile_namespace(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT(walker.isValid());
    moduleEntry.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstNamespaceClass, 1);

    // get namespace identifer
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get namespace access level
    lyric_parser::AccessType access;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstAccessType, access);

    // get namespace block
    auto child = walker.getChild(0);
    moduleEntry.checkClassOrThrow(child, lyric_schema::kLyricAstBlockClass);

    // declare the namespace
    lyric_assembler::NamespaceSymbol *namespaceSymbol;
    TU_ASSIGN_OR_RETURN (namespaceSymbol, block->declareNamespace(
        identifier, lyric_compiler::internal::convert_access_type(access)));

    // compile the namespace block
    TU_RETURN_IF_STATUS (compile_block(namespaceSymbol->namespaceBlock(), child, moduleEntry));

    return {};
}
