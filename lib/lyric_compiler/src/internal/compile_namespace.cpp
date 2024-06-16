
#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/symbol_cache.h>
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

    // get namespace block
    auto child = walker.getChild(0);
    moduleEntry.checkClassOrThrow(child, lyric_schema::kLyricAstBlockClass);

    auto *state = block->blockState();

    // declare the namespace
    lyric_common::SymbolUrl nsUrl;
    TU_ASSIGN_OR_RETURN (nsUrl, block->declareNamespace(identifier, lyric_object::AccessType::Public));

    // get the namespace symbol
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(nsUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::NAMESPACE)
        block->throwAssemblerInvariant("invalid namespace symbol");
    auto *namespaceSymbol = cast_symbol_to_namespace(symbol);

    // compile the namespace block
    TU_RETURN_IF_STATUS (compile_block(namespaceSymbol->namespaceBlock(), child, moduleEntry));

    return {};
}
