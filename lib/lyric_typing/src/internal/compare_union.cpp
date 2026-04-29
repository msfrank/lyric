
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/internal/compare_concrete.h>
#include <lyric_typing/internal/compare_placeholder.h>
#include <lyric_typing/internal/compare_union.h>
#include <lyric_typing/typing_result.h>


tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_concrete_to_union(
    const lyric_common::TypeDef &toUnion,
    const lyric_common::TypeDef &fromConcrete,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toUnion.getType() == lyric_common::TypeDefType::Union);
    TU_ASSERT (fromConcrete.getType() == lyric_common::TypeDefType::Concrete);
    auto *typeCache = state->typeCache();

    lyric_assembler::TypeSignature fromSig;
    TU_ASSIGN_OR_RETURN (fromSig, typeCache->resolveSignature(fromConcrete.getConcreteUrl()));

    for (auto it = toUnion.unionMembersBegin(); it != toUnion.unionMembersEnd(); it++) {
        const auto &memberType = *it;

        lyric_assembler::TypeSignature toSig;
        switch (memberType.getType()) {
            case lyric_common::TypeDefType::Concrete: {
                TU_ASSIGN_OR_RETURN (toSig, typeCache->resolveSignature(memberType.getConcreteUrl()));
                break;
            }
            case lyric_common::TypeDefType::Placeholder: {
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kInvalidType,
                    "union type {} contains invalid member {}", toUnion.toString(), memberType.toString());
        }

        auto cmp = fromSig.compare(toSig);
        switch (cmp) {
            case lyric_runtime::TypeComparison::EQUAL:
            case lyric_runtime::TypeComparison::EXTENDS:
                return cmp;
            default:
                break;
        }
    }

    return lyric_runtime::TypeComparison::DISJOINT;
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_placeholder_to_union(
    const lyric_common::TypeDef &toUnion,
    const lyric_common::TypeDef &fromPlaceholder,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toUnion.getType() == lyric_common::TypeDefType::Union);
    TU_ASSERT (fromPlaceholder.getType() == lyric_common::TypeDefType::Placeholder);

    for (auto it = toUnion.unionMembersBegin(); it != toUnion.unionMembersEnd(); it++) {
        const auto &memberType = *it;

        lyric_runtime::TypeComparison cmp = lyric_runtime::TypeComparison::DISJOINT;

        switch (memberType.getType()) {
            case lyric_common::TypeDefType::Concrete: {
                break;
            }
            case lyric_common::TypeDefType::Placeholder: {
                TU_ASSIGN_OR_RETURN (cmp, compare_placeholder_to_placeholder(
                    memberType, fromPlaceholder, state));
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kInvalidType,
                    "union type {} contains invalid member {}", toUnion.toString(), memberType.toString());
        }

        switch (cmp) {
            case lyric_runtime::TypeComparison::EQUAL:
            case lyric_runtime::TypeComparison::EXTENDS:
                return cmp;
            default:
                break;
        }
    }

    return lyric_runtime::TypeComparison::DISJOINT;
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_union_to_union(
    const lyric_common::TypeDef &toUnion,
    const lyric_common::TypeDef &fromUnion,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toUnion.getType() == lyric_common::TypeDefType::Union);
    TU_ASSERT (fromUnion.getType() == lyric_common::TypeDefType::Union);
    auto *symbolCache = state->symbolCache();

    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_common::TypeDef> toBaseMap;
    for (auto it = toUnion.unionMembersBegin(); it != toUnion.unionMembersEnd(); it++) {
        lyric_common::SymbolUrl toBaseUrl;
        switch (it->getType()) {
            case lyric_common::TypeDefType::Concrete:
                toBaseUrl = it->getConcreteUrl();
                break;
            case lyric_common::TypeDefType::Placeholder:
                toBaseUrl = it->getPlaceholderTemplateUrl();
                break;
            default:
                return TypingStatus::forCondition(TypingCondition::kInvalidType,
                    "{} contains invalid union member {}", toUnion.toString(), it->toString());
        }
        if (toBaseMap.contains(toBaseUrl))
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "{} contains duplicate union member {}", toUnion.toString(), it->toString());
        toBaseMap[toBaseUrl] = *it;
    }

    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_common::TypeDef> fromBaseMap;
    for (auto it = fromUnion.unionMembersBegin(); it != fromUnion.unionMembersEnd(); it++) {
        lyric_common::SymbolUrl fromBaseUrl;
        switch (it->getType()) {
            case lyric_common::TypeDefType::Concrete:
                fromBaseUrl = it->getConcreteUrl();
                break;
            case lyric_common::TypeDefType::Placeholder:
                fromBaseUrl = it->getPlaceholderTemplateUrl();
                break;
            default:
                return TypingStatus::forCondition(TypingCondition::kInvalidType,
                    "{} contains invalid union member {}", fromUnion.toString(), it->toString());
        }
        if (fromBaseMap.contains(fromBaseUrl))
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "{} contains duplicate union member {}", fromUnion.toString(), it->toString());
        fromBaseMap[fromBaseUrl] = *it;
    }

    lyric_runtime::TypeComparison result = lyric_runtime::TypeComparison::DISJOINT;

    for (const auto &fromBase : fromBaseMap) {

        // toUnion contains the exact type of the fromMember, no further checks needed
        if (toBaseMap.contains(fromBase.first)) {
            auto toBase = toBaseMap.extract(fromBase.first);
            lyric_runtime::TypeComparison cmp;
            TU_ASSIGN_OR_RETURN (cmp, compare_assignable(toBase.mapped(), fromBase.second, state));
            switch (cmp) {
                case lyric_runtime::TypeComparison::EQUAL:
                case lyric_runtime::TypeComparison::EXTENDS: {
                    if (result == lyric_runtime::TypeComparison::DISJOINT
                      || result == lyric_runtime::TypeComparison::EQUAL)
                        result = cmp;
                    break;
                }
                default:
                    return lyric_runtime::TypeComparison::DISJOINT;
            }
            continue;
        }

        // exact arg type is not present in the param type union, so check for sealed supertype
        lyric_assembler::AbstractSymbol *fromSym;
        TU_ASSIGN_OR_RETURN (fromSym, symbolCache->getOrImportSymbol(fromBase.first));

        lyric_assembler::TypeHandle *fromTypeHandle;
        switch (fromSym->getSymbolType()) {
            case lyric_assembler::SymbolType::CLASS:
                fromTypeHandle = lyric_assembler::cast_symbol_to_class(fromSym)->classType();
                break;
            case lyric_assembler::SymbolType::ENUM:
                fromTypeHandle = lyric_assembler::cast_symbol_to_enum(fromSym)->enumType();
                break;
            case lyric_assembler::SymbolType::EXISTENTIAL:
                fromTypeHandle = lyric_assembler::cast_symbol_to_existential(fromSym)->existentialType();
                break;
            case lyric_assembler::SymbolType::INSTANCE:
                fromTypeHandle = lyric_assembler::cast_symbol_to_instance(fromSym)->instanceType();
                break;
            case lyric_assembler::SymbolType::STRUCT:
                fromTypeHandle = lyric_assembler::cast_symbol_to_struct(fromSym)->structType();
                break;
            default:
                return TypingStatus::forCondition(TypingCondition::kInvalidType,"invalid symbol type");
        }

        // FIXME: check for sealed marker, return more appropriate status
        auto *fromSupertypeHandle = fromTypeHandle->getSuperType();
        if (fromSupertypeHandle == nullptr)
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "source type has unknown union member");

        auto fromSuperType = fromSupertypeHandle->getTypeDef();
        auto toBase = toBaseMap.extract(fromSuperType.getConcreteUrl());
        if (toBase.empty())
            return lyric_runtime::TypeComparison::DISJOINT;

        lyric_runtime::TypeComparison cmp;
        TU_ASSIGN_OR_RETURN (cmp, compare_concrete_to_concrete(toBase.mapped(), fromBase.second, state));
        switch (cmp) {
            case lyric_runtime::TypeComparison::EQUAL:
            case lyric_runtime::TypeComparison::EXTENDS: {
                if (result == lyric_runtime::TypeComparison::DISJOINT
                  || result == lyric_runtime::TypeComparison::EQUAL)
                    result = cmp;
                break;
            }
            default:
                return lyric_runtime::TypeComparison::DISJOINT;
        }
    }

    return result;
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_union(
    const lyric_common::TypeDef &toUnion,
    const lyric_common::TypeDef &fromType,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toUnion.getType() == lyric_common::TypeDefType::Union);

    switch (fromType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return compare_concrete_to_union(toUnion, fromType, state);
        case lyric_common::TypeDefType::Placeholder:
            return compare_placeholder_to_union(toUnion, fromType, state);
        case lyric_common::TypeDefType::Union:
            return compare_union_to_union(toUnion, fromType, state);
        default:
            return lyric_runtime::TypeComparison::DISJOINT;
    }
}
