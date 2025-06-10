
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
    lyric_assembler::ObjectState *state)
{
    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;

    // if toConcept is not a concept then the types are disjoint
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(toConcept.getConcreteUrl()));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        return lyric_runtime::TypeComparison::DISJOINT;

    // otherwise check if fromConcrete symbol implements the concept
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(fromConcrete.getConcreteUrl()));
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
    lyric_assembler::ObjectState *state)
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
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toConcrete.getType() == lyric_common::TypeDefType::Concrete);
    TU_ASSERT (fromUnion.getType() == lyric_common::TypeDefType::Union);
    auto *typeCache = state->typeCache();

    auto resolveToSignatureResult = typeCache->resolveSignature(toConcrete.getConcreteUrl());
    if (resolveToSignatureResult.isStatus())
        return resolveToSignatureResult.getStatus();
    auto toSig = resolveToSignatureResult.getResult();

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
                return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kInvalidType,
                    "union type {} has invalid member {}", fromUnion.toString(), memberType.toString());
        }

        auto resolveMemberSignatureResult = state->typeCache()->resolveSignature(iterator->getConcreteUrl());
        if (resolveMemberSignatureResult.isStatus())
            return resolveMemberSignatureResult.getStatus();
        auto memberSig = resolveMemberSignatureResult.getResult();

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

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_concrete(
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
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "{} is incompatible with concrete type {}", fromType.toString(), toConcrete.toString());
    }
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_placeholder_to_placeholder(
    const lyric_common::TypeDef &toPlaceholder,
    const lyric_common::TypeDef &fromPlaceholder,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toPlaceholder.getType() == lyric_common::TypeDefType::Placeholder);
    TU_ASSERT (fromPlaceholder.getType() == lyric_common::TypeDefType::Placeholder);

    if (fromPlaceholder.getPlaceholderIndex() != toPlaceholder.getPlaceholderIndex())
        return lyric_runtime::TypeComparison::DISJOINT;

    return fromPlaceholder.getPlaceholderTemplateUrl() == toPlaceholder.getPlaceholderTemplateUrl()?
        lyric_runtime::TypeComparison::EQUAL : lyric_runtime::TypeComparison::DISJOINT;
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_placeholder(
    const lyric_common::TypeDef &toPlaceholder,
    const lyric_common::TypeDef &fromType,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toPlaceholder.getType() == lyric_common::TypeDefType::Placeholder);
    switch (fromType.getType()) {
        case lyric_common::TypeDefType::Placeholder:
            return compare_placeholder_to_placeholder(toPlaceholder, fromType, state);
        default:
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "{} is incompatible with placeholder type {}", fromType.toString(), toPlaceholder.toString());
    }
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_concrete_to_union(
    const lyric_common::TypeDef &toUnion,
    const lyric_common::TypeDef &fromConcrete,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toUnion.getType() == lyric_common::TypeDefType::Union);
    TU_ASSERT (fromConcrete.getType() == lyric_common::TypeDefType::Concrete);

    auto resolveFromSignatureResult = state->typeCache()->resolveSignature(fromConcrete.getConcreteUrl());
    if (resolveFromSignatureResult.isStatus())
        return resolveFromSignatureResult.getStatus();

    auto fromSig = resolveFromSignatureResult.getResult();
    for (auto iterator = toUnion.unionMembersBegin(); iterator != toUnion.unionMembersEnd(); iterator++) {
        const auto &memberType = *iterator;

        lyric_assembler::TypeSignature toSig;
        switch (memberType.getType()) {
            case lyric_common::TypeDefType::Concrete: {
                TU_ASSIGN_OR_RETURN (toSig, state->typeCache()->resolveSignature(memberType.getConcreteUrl()));
                break;
            }
            case lyric_common::TypeDefType::Placeholder: {
                break;
            }
            default:
                return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kInvalidType,
                    "union type {} contains invalid member {}", toUnion.toString(), memberType.toString());
        }

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
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toUnion.getType() == lyric_common::TypeDefType::Union);
    TU_ASSERT (fromUnion.getType() == lyric_common::TypeDefType::Union);

    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_common::TypeDef> toBaseMap;
    for (auto iterator = toUnion.unionMembersBegin(); iterator != toUnion.unionMembersEnd(); iterator++) {
        lyric_common::SymbolUrl toBaseUrl;
        switch (iterator->getType()) {
            case lyric_common::TypeDefType::Concrete:
                toBaseUrl = iterator->getConcreteUrl();
                break;
            case lyric_common::TypeDefType::Placeholder:
                toBaseUrl = iterator->getPlaceholderTemplateUrl();
                break;
            default:
                return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kInvalidType,
                    "{} contains invalid union member {}", toUnion.toString(), iterator->toString());
        }
        if (toBaseMap.contains(toBaseUrl))
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kInvalidType,
                "{} contains duplicate union member {}", toUnion.toString(), iterator->toString());
        toBaseMap[toBaseUrl] = *iterator;
    }

    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_common::TypeDef> fromBaseMap;
    for (auto iterator = fromUnion.unionMembersBegin(); iterator != fromUnion.unionMembersEnd(); iterator++) {
        lyric_common::SymbolUrl fromBaseUrl;
        switch (iterator->getType()) {
            case lyric_common::TypeDefType::Concrete:
                fromBaseUrl = iterator->getConcreteUrl();
                break;
            case lyric_common::TypeDefType::Placeholder:
                fromBaseUrl = iterator->getPlaceholderTemplateUrl();
                break;
            default:
                return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kInvalidType,
                    "{} contains invalid union member {}", fromUnion.toString(), iterator->toString());
        }
        if (fromBaseMap.contains(fromBaseUrl))
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kInvalidType,
                "{} contains duplicate union member {}", fromUnion.toString(), iterator->toString());
        fromBaseMap[fromBaseUrl] = *iterator;
    }

    lyric_runtime::TypeComparison result = lyric_runtime::TypeComparison::DISJOINT;

    for (const auto &fromBase : fromBaseMap) {

        // toUnion contains the exact type of the fromMember, no further checks needed
        if (toBaseMap.contains(fromBase.first)) {
            auto toBase = toBaseMap.extract(fromBase.first);
            auto comparisonResult = lyric_typing::compare_assignable(toBase.mapped(), fromBase.second, state);
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
        lyric_assembler::AbstractSymbol *fromSym;
        TU_ASSIGN_OR_RETURN (fromSym, state->symbolCache()->getOrImportSymbol(fromBase.first));

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
                return lyric_typing::TypingStatus::forCondition(
                    lyric_typing::TypingCondition::kInvalidType,"invalid symbol type");
        }

        // FIXME: check for sealed marker, return more appropriate status
        auto *fromSupertypeHandle = fromTypeHandle->getSuperType();
        if (fromSupertypeHandle == nullptr)
            return lyric_typing::TypingStatus::forCondition(
                lyric_typing::TypingCondition::kInvalidType,
                "source type has unknown union member");

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
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toUnion.getType() == lyric_common::TypeDefType::Union);
    switch (fromType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return compare_concrete_to_union(toUnion, fromType, state);
        case lyric_common::TypeDefType::Union:
            return compare_union_to_union(toUnion, fromType, state);
        default:
            return lyric_typing::TypingStatus::forCondition(
                lyric_typing::TypingCondition::kIncompatibleType,
                "{} is incompatible with union type {}", fromType.toString(), toUnion.toString());
    }
}

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_intersection(
    const lyric_common::TypeDef &toIntersection,
    const lyric_common::TypeDef &fromType,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toIntersection.getType() == lyric_common::TypeDefType::Intersection);
    return lyric_typing::TypingStatus::forCondition(
        lyric_typing::TypingCondition::kIncompatibleType,
        "{} is incompatible with intersection type {}", fromType.toString(), toIntersection.toString());
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::compare_assignable(
    const lyric_common::TypeDef &toRef,
    const lyric_common::TypeDef &fromRef,
    lyric_assembler::ObjectState *state)
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
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "invalid assignable type {}", toRef.toString());
    }
}

tempo_utils::Result<bool>
lyric_typing::is_implementable(
    const lyric_common::TypeDef &toConcept,
    const lyric_common::TypeDef &fromRef,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toConcept.isValid());
    TU_ASSERT (fromRef.isValid());
    TU_ASSERT (state != nullptr);

    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *toSym;
    TU_ASSIGN_OR_RETURN (toSym, symbolCache->getOrImportSymbol(toConcept.getConcreteUrl()));
    if (toSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
            "incompatible type {}; expected concept", toConcept.toString());

    lyric_assembler::AbstractSymbol *fromSym;
    TU_ASSIGN_OR_RETURN (fromSym, symbolCache->getOrImportSymbol(fromRef.getConcreteUrl()));

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
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "type {} cannot implement {}", fromRef.toString(), toConcept.toString());
    }

    if (implHandle == nullptr)
        return false;

    auto implType = implHandle->implType()->getTypeDef();
    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, compare_assignable(implType, fromRef, state));
    return (cmp == lyric_runtime::TypeComparison::EQUAL || cmp == lyric_runtime::TypeComparison::EXTENDS);
}