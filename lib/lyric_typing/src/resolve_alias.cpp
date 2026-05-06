
#include <lyric_typing/resolve_alias.h>

#include "lyric_assembler/class_symbol.h"
#include "lyric_assembler/concept_symbol.h"
#include "lyric_assembler/enum_symbol.h"
#include "lyric_assembler/existential_symbol.h"
#include "lyric_assembler/instance_symbol.h"
#include "lyric_assembler/struct_symbol.h"
#include "lyric_assembler/symbol_cache.h"
#include "lyric_typing/typing_result.h"

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::resolve_impl_alias(
    const lyric_common::TypeDef &fromRef,
    const lyric_common::TypeDef &implType,
    int placeholderIndex,
    lyric_assembler::ObjectState *state)
{
    if (fromRef.getType() != lyric_common::TypeDefType::Concrete)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "cannot resolve alias type for ref {}", fromRef.toString());
    auto refUrl = fromRef.getConcreteUrl();

    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(refUrl));

    lyric_assembler::ImplHandle *implHandle;
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = lyric_assembler::cast_symbol_to_class(symbol);
            implHandle = classSymbol->getImpl(implType);
            break;
        }
        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = lyric_assembler::cast_symbol_to_concept(symbol);
            implHandle = conceptSymbol->getImpl(implType);
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = lyric_assembler::cast_symbol_to_enum(symbol);
            implHandle = enumSymbol->getImpl(implType);
            break;
        }
        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = lyric_assembler::cast_symbol_to_existential(symbol);
            implHandle = existentialSymbol->getImpl(implType);
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = lyric_assembler::cast_symbol_to_instance(symbol);
            implHandle = instanceSymbol->getImpl(implType);
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = lyric_assembler::cast_symbol_to_struct(symbol);
            implHandle = structSymbol->getImpl(implType);
            break;
        }
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "cannot resolve alias type for symbol {}", symbol->getSymbolUrl().toString());
    }

    auto *conceptSymbol = implHandle->implConcept();
    auto *templateHandle = conceptSymbol->conceptTemplate();
    if (templateHandle == nullptr)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "cannot resolve alias type for concept {}", conceptSymbol->getSymbolUrl().toString());

    auto tp = templateHandle->getTemplateParameter(placeholderIndex);
    if (!tp.isAlias)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "cannot resolve alias type for concept {}; type parameter '{}' is not an alias",
            conceptSymbol->getSymbolUrl().toString(), tp.name);

    auto contract = implHandle->getContract();
    auto implementationType = contract.getImplementationType();
    return implementationType.getConcreteArgument(tp.index);
}
