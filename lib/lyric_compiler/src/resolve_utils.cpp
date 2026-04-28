
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/resolve_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/member_reifier.h>

tempo_utils::Status
lyric_compiler::resolve_name(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::DataReference &ref)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    TU_ASSIGN_OR_RETURN (ref, block->resolveReference(identifier));
    return {};
}

tempo_utils::Status
lyric_compiler::resolve_member(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::DataReference &ref,
    const std::vector<DerefEffect> &effects,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (block != nullptr);
    TU_ASSERT (driver != nullptr);

    if (effects.empty())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid member resolve; effects vector is empty");

    const auto &effect = effects.back();
    const auto receiverType = effect.receiverType;
    auto *derefSymbol = effect.derefSymbol;

    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    auto thisReceiver = current_ref_is_this_receiver(symbolCache, block, effects);

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    lyric_typing::MemberReifier reifier(typeSystem);
    switch (derefSymbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = lyric_assembler::cast_symbol_to_class(derefSymbol);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType, classSymbol->classTemplate()));
            TU_ASSIGN_OR_RETURN (ref, classSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            return {};
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = lyric_assembler::cast_symbol_to_enum(derefSymbol);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
            TU_ASSIGN_OR_RETURN (ref, enumSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            return {};
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = lyric_assembler::cast_symbol_to_instance(derefSymbol);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
            TU_ASSIGN_OR_RETURN (ref, instanceSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            return {};
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = lyric_assembler::cast_symbol_to_struct(derefSymbol);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
            TU_ASSIGN_OR_RETURN (ref, structSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            return {};
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid receiver symbol {}", derefSymbol->getSymbolUrl().toString());
    }
}
