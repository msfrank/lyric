
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_typing/resolve_assignable.h>
#include <lyric_typing/resolve_implementable.h>
#include <lyric_typing/typing_result.h>

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::resolve_implementable(
    const TypeSpec &assignable,
    lyric_assembler::AbstractResolver *resolver,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (assignable.isValid());
    TU_ASSERT (resolver != nullptr);
    TU_ASSERT (state != nullptr);
    auto *symbolCache = state->symbolCache();

    if (assignable.getType() != TypeSpecType::Singular)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "invalid implementable type spec {}", assignable.toString());

    lyric_common::SymbolUrl definitionUrl;
    TU_ASSIGN_OR_RETURN (definitionUrl, resolver->resolveDefinition(assignable.getTypePath()));

    lyric_assembler::ConceptSymbol *conceptSymbol;
    TU_ASSIGN_OR_RETURN (conceptSymbol, symbolCache->getOrImportConcept(definitionUrl));

    std::vector<lyric_common::TypeDef> typeArguments;

    auto *templateHandle = conceptSymbol->conceptTemplate();
    if (templateHandle != nullptr) {
        int numConsumerParameters = 0;

        for (auto it = templateHandle->templateParametersBegin(); it != templateHandle->templateParametersEnd(); ++it) {
            const auto &tp = *it;
            if (!tp.isAlias) {
                numConsumerParameters++;
            }
        }

        auto implArguments = assignable.getTypeParameters();
        if (implArguments.size() != numConsumerParameters)
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "wrong number of type arguments for {}; expected {} but found {}",
                conceptSymbol->getSymbolUrl().toString(), numConsumerParameters, implArguments.size());

        for (const auto &argType : implArguments) {
            lyric_common::TypeDef typeArgument;
            TU_ASSIGN_OR_RETURN (typeArgument, resolve_assignable(argType, resolver, state));
            typeArguments.push_back(typeArgument);
        }
    }

    return lyric_common::TypeDef::forConcrete(definitionUrl, typeArguments);
}
