
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/internal/compare_concrete.h>
#include <lyric_typing/typing_result.h>

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_concrete_to_concept(
    const lyric_common::TypeDef &toConcept,
    const lyric_common::TypeDef &fromConcrete,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toConcept.getType() == lyric_common::TypeDefType::Concrete);
    TU_ASSERT (fromConcrete.getType() == lyric_common::TypeDefType::Concrete);
    TU_NOTNULL (state);
    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(fromConcrete.getConcreteUrl()));

    // check if fromConcrete symbol implements the concept
    switch (symbol->getSymbolType()) {

        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(symbol);
            if (classSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = lyric_assembler::cast_symbol_to_concept(symbol);
            if (conceptSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = lyric_assembler::cast_symbol_to_enum(symbol);
            if (enumSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = lyric_assembler::cast_symbol_to_existential(symbol);
            if (existentialSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = lyric_assembler::cast_symbol_to_instance(symbol);
            if (instanceSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = lyric_assembler::cast_symbol_to_struct(symbol);
            if (structSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        default:
            return lyric_runtime::TypeComparison::DISJOINT;
    }
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_concrete_to_concrete(
    const lyric_common::TypeDef &toConcrete,
    const lyric_common::TypeDef &fromConcrete,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toConcrete.getType() == lyric_common::TypeDefType::Concrete);
    TU_ASSERT (fromConcrete.getType() == lyric_common::TypeDefType::Concrete);
    auto *symbolCache = state->symbolCache();
    auto *typeCache = state->typeCache();

    // first try to directly compare the two types
    lyric_assembler::TypeSignature toSig;
    TU_ASSIGN_OR_RETURN (toSig, typeCache->resolveSignature(toConcrete.getConcreteUrl()));
    lyric_assembler::TypeSignature fromSig;
    TU_ASSIGN_OR_RETURN (fromSig, typeCache->resolveSignature(fromConcrete.getConcreteUrl()));
    auto directComparison = fromSig.compare(toSig);

    // if comparison result is equal or extends then we are done
    if (directComparison != lyric_runtime::TypeComparison::DISJOINT)
        return directComparison;

    // if toConcrete is a concept then see if fromConcrete implements it
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(toConcrete.getConcreteUrl()));

    // if toConcept is not a concept then the types are disjoint
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        return lyric_runtime::TypeComparison::DISJOINT;

    // otherwise compare against impls
    return compare_concrete_to_concept(toConcrete, fromConcrete, state);
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_union_to_concrete(
    const lyric_common::TypeDef &toConcrete,
    const lyric_common::TypeDef &fromUnion,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toConcrete.getType() == lyric_common::TypeDefType::Concrete);
    TU_ASSERT (fromUnion.getType() == lyric_common::TypeDefType::Union);
    auto *typeCache = state->typeCache();

    lyric_assembler::TypeSignature toSig;
    TU_ASSIGN_OR_RETURN (toSig, typeCache->resolveSignature(toConcrete.getConcreteUrl()));

    for (auto iterator = fromUnion.unionMembersBegin(); iterator != fromUnion.unionMembersEnd(); iterator++) {
        const auto &memberType = *iterator;

        lyric_common::SymbolUrl memberUrl;
        switch (memberType.getType()) {
            case lyric_common::TypeDefType::Concrete: {
                memberUrl = memberType.getConcreteUrl();
                break;
            }
            case lyric_common::TypeDefType::Placeholder: {
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kInvalidType,
                    "union type {} has invalid member {}", fromUnion.toString(), memberType.toString());
        }

        lyric_assembler::TypeSignature memberSig;
        TU_ASSIGN_OR_RETURN (memberSig, typeCache->resolveSignature(iterator->getConcreteUrl()));

        switch (memberSig.compare(toSig)) {
            case lyric_runtime::TypeComparison::EXTENDS:
            case lyric_runtime::TypeComparison::EQUAL:
                break;
            default:
                return lyric_runtime::TypeComparison::DISJOINT;
        }
    }

    return lyric_runtime::TypeComparison::EQUAL;
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_concrete(
    const lyric_common::TypeDef &toConcrete,
    const lyric_common::TypeDef &fromType,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toConcrete.getType() == lyric_common::TypeDefType::Concrete);

    switch (fromType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return compare_concrete_to_concrete(toConcrete, fromType, state);
        case lyric_common::TypeDefType::Union:
            return compare_union_to_concrete(toConcrete, fromType, state);
        default:
            return lyric_runtime::TypeComparison::DISJOINT;
    }
}
