
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/typing_result.h>

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_concrete_to_concept(
    const lyric_common::TypeDef &toConcept,
    const lyric_common::TypeDef &fromConcrete,
    lyric_assembler::AssemblyState *state)
{
    auto *symbolCache = state->symbolCache();

    // if toConcept is not a concept then the types are disjoint
    auto *symbol = symbolCache->getSymbol(toConcept.getConcreteUrl());
    TU_ASSERT (symbol != nullptr);
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        return lyric_runtime::TypeComparison::DISJOINT;

    // otherwise check if fromConcrete symbol implements the concept
    symbol = symbolCache->getSymbol(fromConcrete.getConcreteUrl());
    TU_ASSERT (symbol != nullptr);

    switch (symbol->getSymbolType()) {

        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(symbol);
            if (classSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = cast_symbol_to_concept(symbol);
            if (conceptSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(symbol);
            if (enumSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(symbol);
            if (existentialSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(symbol);
            if (instanceSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(symbol);
            if (structSymbol->hasImpl(toConcept))
                return lyric_runtime::TypeComparison::EQUAL;
            return lyric_runtime::TypeComparison::DISJOINT;
        }

        default:
            return lyric_runtime::TypeComparison::DISJOINT;
    }
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_concrete_to_concrete(
    const lyric_common::TypeDef &toConcrete,
    const lyric_common::TypeDef &fromConcrete,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (toConcrete.getType() == lyric_common::TypeDefType::Concrete);
    TU_ASSERT (fromConcrete.getType() == lyric_common::TypeDefType::Concrete);

    // first try to directly compare the two types
    lyric_assembler::TypeSignature toSig;
    TU_ASSIGN_OR_RETURN (toSig, state->typeCache()->resolveSignature(toConcrete.getConcreteUrl()));
    lyric_assembler::TypeSignature fromSig;
    TU_ASSIGN_OR_RETURN (fromSig, state->typeCache()->resolveSignature(fromConcrete.getConcreteUrl()));
    auto directComparison = fromSig.compare(toSig);

    // if comparison result is equal or extends then we are done
    if (directComparison != lyric_runtime::TypeComparison::DISJOINT)
        return directComparison;

    // otherwise if toConcrete symbol is a concept, then check if fromConcrete implements the concept
    return compare_concrete_to_concept(toConcrete, fromConcrete, state);
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_union_to_concrete(
    const lyric_common::TypeDef &toConcrete,
    const lyric_common::TypeDef &fromUnion,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (toConcrete.getType() == lyric_common::TypeDefType::Concrete);
    TU_ASSERT (fromUnion.getType() == lyric_common::TypeDefType::Union);

    auto resolveToSignatureResult = state->typeCache()->resolveSignature(toConcrete.getConcreteUrl());
    if (resolveToSignatureResult.isStatus())
        return resolveToSignatureResult.getStatus();
    auto toSig = resolveToSignatureResult.getResult();

    lyric_runtime::TypeComparison comparison = lyric_runtime::TypeComparison::DISJOINT;

    for (auto iterator = fromUnion.unionMembersBegin(); iterator != fromUnion.unionMembersEnd(); iterator++) {
        auto resolveMemberSignatureResult = state->typeCache()->resolveSignature(iterator->getConcreteUrl());
        if (resolveMemberSignatureResult.isStatus())
            return resolveMemberSignatureResult.getStatus();
        auto memberSig = resolveMemberSignatureResult.getResult();

        switch (memberSig.compare(toSig)) {
            case lyric_runtime::TypeComparison::EXTENDS: {
                if (comparison == lyric_runtime::TypeComparison::DISJOINT) {
                    comparison = lyric_runtime::TypeComparison::EXTENDS;
                }
                break;
            }
            case lyric_runtime::TypeComparison::EQUAL:
                comparison = lyric_runtime::TypeComparison::EQUAL;
                break;
            default:
                return lyric_runtime::TypeComparison::DISJOINT;
        }
    }

    return comparison;
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_concrete(
    const lyric_common::TypeDef &toConcrete,
    const lyric_common::TypeDef &fromType,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (toConcrete.getType() == lyric_common::TypeDefType::Concrete);
    switch (fromType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return compare_concrete_to_concrete(toConcrete, fromType, state);
        case lyric_common::TypeDefType::Union:
            return compare_union_to_concrete(toConcrete, fromType, state);
        default:
            return state->logAndContinue(lyric_typing::TypingCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "{} is incompatible with concrete type {}", fromType.toString(), toConcrete.toString());
    }
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_placeholder_to_placeholder(
    const lyric_common::TypeDef &toPlaceholder,
    const lyric_common::TypeDef &fromPlaceholder,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (toPlaceholder.getType() == lyric_common::TypeDefType::Placeholder);
    TU_ASSERT (fromPlaceholder.getType() == lyric_common::TypeDefType::Placeholder);

    if (fromPlaceholder.getPlaceholderIndex() == toPlaceholder.getPlaceholderIndex())
            return lyric_runtime::TypeComparison::EQUAL;
    return lyric_runtime::TypeComparison::DISJOINT;
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_placeholder(
    const lyric_common::TypeDef &toPlaceholder,
    const lyric_common::TypeDef &fromType,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (toPlaceholder.getType() == lyric_common::TypeDefType::Placeholder);
    switch (fromType.getType()) {
        case lyric_common::TypeDefType::Placeholder:
            return compare_placeholder_to_placeholder(toPlaceholder, fromType, state);
        default:
            return state->logAndContinue(lyric_typing::TypingCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "{} is incompatible with placeholder type {}", fromType.toString(), toPlaceholder.toString());
    }
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_concrete_to_union(
    const lyric_common::TypeDef &toUnion,
    const lyric_common::TypeDef &fromConcrete,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (toUnion.getType() == lyric_common::TypeDefType::Union);
    TU_ASSERT (fromConcrete.getType() == lyric_common::TypeDefType::Concrete);

    auto resolveFromSignatureResult = state->typeCache()->resolveSignature(fromConcrete.getConcreteUrl());
    if (resolveFromSignatureResult.isStatus())
        return resolveFromSignatureResult.getStatus();

    auto fromSig = resolveFromSignatureResult.getResult();
    for (auto iterator = toUnion.unionMembersBegin(); iterator != toUnion.unionMembersEnd(); iterator++) {
        auto resolveToSignatureResult = state->typeCache()->resolveSignature(iterator->getConcreteUrl());
        if (resolveToSignatureResult.isStatus())
            return resolveToSignatureResult.getStatus();
        auto toSig = resolveToSignatureResult.getResult();

        auto comparison = fromSig.compare(toSig);
        switch (comparison) {
            case lyric_runtime::TypeComparison::EQUAL:
            case lyric_runtime::TypeComparison::EXTENDS:
                return comparison;
            default:
                break;
        }
    }

    return lyric_runtime::TypeComparison::DISJOINT;
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_union_to_union(
    const lyric_common::TypeDef &toUnion,
    const lyric_common::TypeDef &fromUnion,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (toUnion.getType() == lyric_common::TypeDefType::Union);
    TU_ASSERT (fromUnion.getType() == lyric_common::TypeDefType::Union);

    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_common::TypeDef> toBaseMap;
    for (auto iterator = toUnion.unionMembersBegin(); iterator != toUnion.unionMembersEnd(); iterator++) {
        if (iterator->getType() != lyric_common::TypeDefType::Concrete)
            return state->logAndContinue(lyric_typing::TypingCondition::kInvalidType,
                tempo_tracing::LogSeverity::kError,
                "{} contains invalid union member {}", toUnion.toString(), iterator->toString());
        auto toBaseUrl = iterator->getConcreteUrl();
        if (toBaseMap.contains(toBaseUrl))
            return state->logAndContinue(lyric_typing::TypingCondition::kInvalidType,
                tempo_tracing::LogSeverity::kError,
                "{} contains duplicate union member {}", toUnion.toString(), iterator->toString());
        toBaseMap[toBaseUrl] = *iterator;
    }

    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_common::TypeDef> fromBaseMap;
    for (auto iterator = fromUnion.unionMembersBegin(); iterator != fromUnion.unionMembersEnd(); iterator++) {
        if (iterator->getType() != lyric_common::TypeDefType::Concrete)
            return state->logAndContinue(lyric_typing::TypingCondition::kInvalidType,
                tempo_tracing::LogSeverity::kError,
                "{} contains invalid union member {}", fromUnion.toString(), iterator->toString());
        auto fromBaseUrl = iterator->getConcreteUrl();
        if (fromBaseMap.contains(fromBaseUrl))
            return state->logAndContinue(lyric_typing::TypingCondition::kInvalidType,
                tempo_tracing::LogSeverity::kError,
                "{} contains duplicate union member {}", fromUnion.toString(), iterator->toString());
        fromBaseMap[fromBaseUrl] = *iterator;
    }

    lyric_runtime::TypeComparison result = lyric_runtime::TypeComparison::DISJOINT;

    for (const auto &fromBase : fromBaseMap) {

        // toUnion contains the exact type of the fromMember, no further checks needed
        if (toBaseMap.contains(fromBase.first)) {
            auto toBase = toBaseMap.extract(fromBase.first);
            auto comparisonResult = compare_concrete_to_concrete(toBase.mapped(), fromBase.second, state);
            if (comparisonResult.isStatus())
                return comparisonResult;
            auto comparison = comparisonResult.getResult();
            switch (comparison) {
                case lyric_runtime::TypeComparison::EQUAL:
                case lyric_runtime::TypeComparison::EXTENDS: {
                    if (result == lyric_runtime::TypeComparison::DISJOINT
                      || result == lyric_runtime::TypeComparison::EQUAL)
                        result = comparison;
                    break;
                }
                default:
                    return lyric_runtime::TypeComparison::DISJOINT;
            }
            continue;
        }

        // exact arg type is not present in the param type union, so check for sealed supertype
        if (!state->symbolCache()->hasSymbol(fromBase.first))
            return state->logAndContinue(lyric_typing::TypingCondition::kInvalidType,
                tempo_tracing::LogSeverity::kError,
                "{} contains unknown union member {}", fromUnion.toString(), fromBase.first.toString());
        auto *fromSym = state->symbolCache()->getSymbol(fromBase.first);

        lyric_assembler::TypeHandle *fromTypeHandle;
        switch (fromSym->getSymbolType()) {
            case lyric_assembler::SymbolType::CLASS:
                fromTypeHandle = cast_symbol_to_class(fromSym)->classType();
                break;
            case lyric_assembler::SymbolType::ENUM:
                fromTypeHandle = cast_symbol_to_enum(fromSym)->enumType();
                break;
            case lyric_assembler::SymbolType::EXISTENTIAL:
                fromTypeHandle = cast_symbol_to_existential(fromSym)->existentialType();
                break;
            case lyric_assembler::SymbolType::INSTANCE:
                fromTypeHandle = cast_symbol_to_instance(fromSym)->instanceType();
                break;
            case lyric_assembler::SymbolType::STRUCT:
                fromTypeHandle = cast_symbol_to_struct(fromSym)->structType();
                break;
            default:
                state->throwAssemblerInvariant("invalid symbol type");
        }

        // FIXME: check for sealed marker, return more appropriate status
        auto *fromSupertypeHandle = fromTypeHandle->getSuperType();
        if (fromSupertypeHandle == nullptr)
            state->throwAssemblerInvariant("source type has unknown union member");

        auto fromSuperType = fromSupertypeHandle->getTypeDef();
        auto toBase = toBaseMap.extract(fromSuperType.getConcreteUrl());
        if (toBase.empty())
            return lyric_runtime::TypeComparison::DISJOINT;
        auto comparisonResult = compare_concrete_to_concrete(toBase.mapped(), fromBase.second, state);
        if (comparisonResult.isStatus())
            return comparisonResult;
        auto comparison = comparisonResult.getResult();
        switch (comparison) {
            case lyric_runtime::TypeComparison::EQUAL:
            case lyric_runtime::TypeComparison::EXTENDS: {
                if (result == lyric_runtime::TypeComparison::DISJOINT
                  || result == lyric_runtime::TypeComparison::EQUAL)
                    result = comparison;
                break;
            }
            default:
                return lyric_runtime::TypeComparison::DISJOINT;
        }
    }

    return result;
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_union(
    const lyric_common::TypeDef &toUnion,
    const lyric_common::TypeDef &fromType,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (toUnion.getType() == lyric_common::TypeDefType::Union);
    switch (fromType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return compare_concrete_to_union(toUnion, fromType, state);
        case lyric_common::TypeDefType::Union:
            return compare_union_to_union(toUnion, fromType, state);
        default:
            return state->logAndContinue(lyric_typing::TypingCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "{} is incompatible with union type {}", fromType.toString(), toUnion.toString());
    }
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_intersection(
    const lyric_common::TypeDef &toIntersection,
    const lyric_common::TypeDef &fromType,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (toIntersection.getType() == lyric_common::TypeDefType::Intersection);
    return state->logAndContinue(lyric_typing::TypingCondition::kIncompatibleType,
        tempo_tracing::LogSeverity::kError,
        "{} is incompatible with intersection type {}", fromType.toString(), toIntersection.toString());
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::compare_assignable(
    const lyric_common::TypeDef &toRef,
    const lyric_common::TypeDef &fromRef,
    lyric_assembler::AssemblyState *state)
{
 /*
  *
  *                       || to Concrete (simple) || to Concrete (parametric) || to Placeholder || to Intersection || to Union ||
  *                       -------------------------------------------------------------------------------------------------------
  *     Concrete (simple) || yes                     no                          no                yes                yes
  * Concrete (parametric) || no                      yes                         no                yes                yes
  *           Placeholder || no                      no                          yes               no                 no
  *          Intersection || no                      no                          no                yes                no
  *                 Union || no                      no                          no                no                 yes
  */

    switch (toRef.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return compare_concrete(toRef, fromRef, state);
        case lyric_common::TypeDefType::Placeholder:
            return compare_placeholder(toRef, fromRef, state);
        case lyric_common::TypeDefType::Union:
            return compare_union(toRef, fromRef, state);
        case lyric_common::TypeDefType::Intersection:
            return compare_intersection(toRef, fromRef, state);
        default:
            return state->logAndContinue(TypingCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "invalid assignable type {}", toRef.toString());
    }
}

tempo_utils::Result<bool>
lyric_typing::is_implementable(
    const lyric_common::TypeDef &toConcept,
    const lyric_common::TypeDef &fromRef,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (toConcept.isValid());
    TU_ASSERT (fromRef.isValid());
    TU_ASSERT (state != nullptr);

    auto *symbolCache = state->symbolCache();

    auto *toSym = symbolCache->getSymbol(toConcept.getConcreteUrl());
    if (toSym == nullptr)
        return state->logAndContinue(TypingCondition::kMissingType,
            tempo_tracing::LogSeverity::kError,
            "missing symbol for concept type {}", toConcept.toString());
    if (toSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        return state->logAndContinue(TypingCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "incompatible type {}; expected concept", toConcept.toString());

    auto *fromSym = symbolCache->getSymbol(fromRef.getConcreteUrl());
    if (fromSym == nullptr)
        return state->logAndContinue(TypingCondition::kMissingType,
            tempo_tracing::LogSeverity::kError,
            "missing symbol for type {}", fromRef.toString());

    auto toUrl = toConcept.getConcreteUrl();
    lyric_assembler::ImplHandle *implHandle = nullptr;

    switch (fromSym->getSymbolType()) {

        // special case: if fromRef is a concept, then directly compare the types and return
        case lyric_assembler::SymbolType::CONCEPT: {
            lyric_runtime::TypeComparison cmp;
            TU_ASSIGN_OR_RETURN (cmp, compare_assignable(toConcept, fromRef, state));
            return (cmp == lyric_runtime::TypeComparison::EQUAL || cmp == lyric_runtime::TypeComparison::EXTENDS);
        }

        // otherwise if fromRef is not a concept, then check the symbol for a matching impl
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(fromSym);
            implHandle = classSymbol->getImpl(toUrl);
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(fromSym);
            implHandle = enumSymbol->getImpl(toUrl);
            break;
        }
        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(fromSym);
            implHandle = existentialSymbol->getImpl(toUrl);
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(fromSym);
            implHandle = instanceSymbol->getImpl(toUrl);
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(fromSym);
            implHandle = structSymbol->getImpl(toUrl);
            break;
        }
        default:
            return state->logAndContinue(TypingCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "type {} cannot implement {}", fromRef.toString(), toConcept.toString());
    }

    if (implHandle == nullptr)
        return false;

    auto implType = implHandle->implType()->getTypeDef();
    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, compare_assignable(implType, fromRef, state));
    return (cmp == lyric_runtime::TypeComparison::EQUAL || cmp == lyric_runtime::TypeComparison::EXTENDS);
}