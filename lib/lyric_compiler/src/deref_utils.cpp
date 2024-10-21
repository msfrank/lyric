
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/resolve_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/member_reifier.h>

tempo_utils::Status
lyric_compiler::deref_this(
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (driver != nullptr);

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, block->resolveReference("$this"));

    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();

    // ensure symbol for $this is imported
    TU_RETURN_IF_STATUS(symbolCache->getOrImportSymbol(ref.symbolUrl));

    // load the receiver
    TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));

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
lyric_compiler::deref_name(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (block != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (driver != nullptr);

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, resolve_binding(node, block));
    TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));

    return driver->pushResult(ref.typeDef);
}

tempo_utils::Status
lyric_compiler::deref_name(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle **blockptr,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (blockptr != nullptr && *blockptr != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (driver != nullptr);

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, resolve_binding(node, *blockptr));

    auto *state = (*blockptr)->blockState();
    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));

    if (symbol->getSymbolType() == lyric_assembler::SymbolType::NAMESPACE) {
        auto *namespaceSymbol = cast_symbol_to_namespace(symbol);
        *blockptr = namespaceSymbol->namespaceBlock();
    } else {
        TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));
    }

    return driver->pushResult(ref.typeDef);
}

tempo_utils::Status
lyric_compiler::deref_member(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (block != nullptr);
    TU_ASSERT (fragment != nullptr);
    TU_ASSERT (receiverType.isValid());
    TU_ASSERT (driver != nullptr);

    auto *state = driver->getState();
    auto *typeSystem = driver->getTypeSystem();
    auto *symbolCache = state->symbolCache();

    auto receiverUrl = receiverType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *receiver;
    TU_ASSIGN_OR_RETURN (receiver, symbolCache->getOrImportSymbol(receiverUrl));

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    lyric_assembler::DataReference ref;
    switch (receiver->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType, classSymbol->classTemplate());
            TU_ASSIGN_OR_RETURN (ref, classSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            TU_ASSIGN_OR_RETURN (ref, enumSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            TU_ASSIGN_OR_RETURN (ref, instanceSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            TU_ASSIGN_OR_RETURN (ref, structSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        default:
            block->throwAssemblerInvariant("invalid receiver symbol {}", receiverUrl.toString());
    }

    TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));

    // drop the previous result from the stack
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // push member result
    return driver->pushResult(ref.typeDef);
}
