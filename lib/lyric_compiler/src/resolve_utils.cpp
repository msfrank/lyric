
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/resolve_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/member_reifier.h>

tempo_utils::Status
lyric_compiler::resolve_name(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::DataReference &ref,
    CompilerScanDriver *driver)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    TU_ASSIGN_OR_RETURN (ref, block->resolveReference(identifier));
    return {};
}

tempo_utils::Status
lyric_compiler::resolve_member(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver,
    lyric_assembler::DataReference &ref,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (block != nullptr);
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

    switch (receiver->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType, classSymbol->classTemplate());
            TU_ASSIGN_OR_RETURN (ref, classSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            return {};
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            TU_ASSIGN_OR_RETURN (ref, enumSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            return {};
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            TU_ASSIGN_OR_RETURN (ref, instanceSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            return {};
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            TU_ASSIGN_OR_RETURN (ref, structSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            return {};
        }
        default:
            return block->logAndContinue(CompilerCondition::kCompilerInvariant,
                tempo_tracing::LogSeverity::kError,
                "invalid receiver symbol {}", receiverUrl.toString());
    }
}
