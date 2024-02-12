
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_import.h>
#include <lyric_object/object_walker.h>
#include <lyric_parser/ast_attrs.h>
#include <tempo_utils/log_stream.h>

tempo_utils::Status
lyric_compiler::internal::compile_import_module(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *state = moduleEntry.getState();
    auto *importCache = state->importCache();

    // get the namespace identifier
    std::string namespaceIdentifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, namespaceIdentifier);

    // declare the namespace
    auto declareNamespaceResult = block->declareNamespace(namespaceIdentifier, lyric_object::AccessType::Public);
    if (declareNamespaceResult.isStatus())
        return declareNamespaceResult.getStatus();
    auto nsUrl = declareNamespaceResult.getResult();
    if (!state->symbolCache()->hasSymbol(nsUrl))
        block->throwAssemblerInvariant("missing namespace symbol {}", nsUrl.toString());
    auto *nsSym = state->symbolCache()->getSymbol(nsUrl);
    if (nsSym->getSymbolType() != lyric_assembler::SymbolType::NAMESPACE)
        block->throwAssemblerInvariant("invalid namespace symbol {}", nsUrl.toString());
    auto *ns = cast_symbol_to_namespace(nsSym);
    auto *nsBlock = ns->namespaceBlock();

    // resolve assembly to absolute location
    lyric_common::AssemblyLocation importLocation;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstAssemblyLocation, importLocation);

    return importCache->importModule(importLocation, nsBlock);
}

tempo_utils::Status
lyric_compiler::internal::compile_import_symbols(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *state = moduleEntry.getState();
    auto *importCache = state->importCache();

    // resolve assembly to absolute location
    lyric_common::AssemblyLocation importLocation;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstAssemblyLocation, importLocation);

    absl::flat_hash_set<lyric_assembler::ImportRef> importRefs;

    // import each specified symbol into the current block
    for (int i = 0; i < walker.numChildren(); i++) {
        auto symbolRef = walker.getChild(i);
        moduleEntry.checkClassOrThrow(symbolRef, lyric_schema::kLyricAstSymbolRefClass);

        lyric_common::SymbolPath symbolPath;
        moduleEntry.parseAttrOrThrow(symbolRef, lyric_parser::kLyricAstSymbolPath, symbolPath);

        importRefs.insert(lyric_assembler::ImportRef(symbolPath));
    }

    return importCache->importModule(importLocation, block, importRefs);
}

tempo_utils::Status
lyric_compiler::internal::compile_import_all(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *state = moduleEntry.getState();
    auto *importCache = state->importCache();

    // resolve assembly to absolute location
    lyric_common::AssemblyLocation importLocation;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstAssemblyLocation, importLocation);

    return importCache->importModule(importLocation, block);
}
