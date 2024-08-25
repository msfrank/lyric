
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_parser/ast_attrs.h>

tempo_utils::Status
lyric_compiler::compile_this(
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (driver != nullptr);

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, block->resolveReference("$this"));

    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();

    // ensure symbol for $this is imported
    TU_RETURN_IF_STATUS(symbolCache->getOrImportSymbol(ref.symbolUrl));

    // load the receiver
    TU_RETURN_IF_NOT_OK (block->load(ref));

    return driver->pushResult(ref.typeDef);
}

inline tempo_utils::Result<lyric_assembler::DataReference>
resolve_binding(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    return block->resolveReference(identifier);
}

tempo_utils::Status
lyric_compiler::compile_name(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (block != nullptr);
    TU_ASSERT (driver != nullptr);

    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, resolve_binding(node, block));
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));

    if (ref.referenceType == lyric_assembler::ReferenceType::Descriptor) {
        lyric_common::SymbolUrl ctorOrInitUrl;

        switch (symbol->getSymbolType()) {
            case lyric_assembler::SymbolType::ENUM:
                ctorOrInitUrl = cast_symbol_to_enum(symbol)->getCtor();
                break;
            case lyric_assembler::SymbolType::INSTANCE:
                ctorOrInitUrl = cast_symbol_to_instance(symbol)->getCtor();
                break;
            case lyric_assembler::SymbolType::STATIC:
                ctorOrInitUrl = cast_symbol_to_static(symbol)->getInitializer();
                break;
            default:
                return block->logAndContinue(CompilerCondition::kMissingVariable,
                    tempo_tracing::LogSeverity::kError,
                    "cannot dereference symbol {}", ref.symbolUrl.toString());
        }

        // ensure that links are created for the symbol and its ctor/initializer
        symbol->touch();
        if (symbolCache->hasSymbol(ctorOrInitUrl)) {
            TU_RETURN_IF_NOT_OK (symbolCache->touchSymbol(ctorOrInitUrl));
        }
    }

    TU_RETURN_IF_NOT_OK (block->load(ref));

    return driver->pushResult(ref.typeDef);
}
