
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/match_utils.h>
#include <lyric_compiler/type_utils.h>
#include <lyric_parser/ast_attrs.h>

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::compile_predicate(
    const lyric_assembler::DataReference &targetRef,
    const lyric_common::TypeDef &predicateType,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (predicateType.isValid());
    auto *typeCache = driver->getTypeCache();
    auto *typeSystem = driver->getTypeSystem();

    // push target variable onto the top of the stack
    TU_RETURN_IF_NOT_OK (fragment->loadRef(targetRef));

    // pop target and push target type descriptor onto the stack
    TU_RETURN_IF_NOT_OK (fragment->invokeTypeOf());

    if (!typeCache->hasType(predicateType))
        return CompilerStatus::forCondition(CompilerCondition::kMissingType,
            "missing predicate type {}", predicateType.toString());

    // determine the match type
    lyric_common::TypeDef matchType;
    switch (predicateType.getType()) {
        case lyric_common::TypeDefType::Concrete: {
            matchType = predicateType;
            break;
        }
        case lyric_common::TypeDefType::Placeholder: {
            std::pair<lyric_object::BoundType,lyric_common::TypeDef> bound;
            TU_ASSIGN_OR_RETURN (bound, typeSystem->resolveBound(predicateType));
            if (bound.first != lyric_object::BoundType::None && bound.first != lyric_object::BoundType::Extends)
                return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                    "predicate type {} cannot match; constraint must have Extends bounds",
                    predicateType.toString());
            matchType = bound.second;
            break;
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid predicate type {}", predicateType.toString());
    }

    // push match type descriptor onto the stack
    TU_RETURN_IF_NOT_OK (fragment->loadType(matchType));

    // verify that the target type can be a subtype of match type
    TU_RETURN_IF_NOT_OK (match_types(targetRef.typeDef, matchType, block, driver));

    // perform type comparison
    TU_RETURN_IF_NOT_OK (fragment->typeCompare());

    return matchType;
}

/**
 *
 * @param targetType
 * @param patchList
 * @return
 */
static tempo_utils::Result<bool>
check_concrete_target_is_exhaustive(
    const lyric_common::TypeDef &targetType,
    const std::vector<lyric_assembler::MatchWhenPatch> &patchList,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
{
    TU_ASSERT (targetType.getType() == lyric_common::TypeDefType::Concrete);
    auto *symbolCache = driver->getSymbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(targetType.getConcreteUrl()));

    lyric_object::DeriveType derive;
    absl::flat_hash_set<lyric_common::TypeDef> targetSet;

    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(symbol);
            derive = classSymbol->getDeriveType();
            if (derive == lyric_object::DeriveType::Sealed) {
                targetSet.insert(classSymbol->sealedTypesBegin(), classSymbol->sealedTypesEnd());
            }
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(symbol);
            derive = enumSymbol->getDeriveType();
            if (derive == lyric_object::DeriveType::Sealed) {
                targetSet.insert(enumSymbol->sealedTypesBegin(), enumSymbol->sealedTypesEnd());
            }
            break;
        }
        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(symbol);
            derive = existentialSymbol->getDeriveType();
            if (derive == lyric_object::DeriveType::Sealed) {
                targetSet.insert(existentialSymbol->sealedTypesBegin(), existentialSymbol->sealedTypesEnd());
            }
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(symbol);
            derive = instanceSymbol->getDeriveType();
            if (derive == lyric_object::DeriveType::Sealed) {
                targetSet.insert(instanceSymbol->sealedTypesBegin(), instanceSymbol->sealedTypesEnd());
            }
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(symbol);
            derive = structSymbol->getDeriveType();
            if (derive == lyric_object::DeriveType::Sealed) {
                targetSet.insert(structSymbol->sealedTypesBegin(), structSymbol->sealedTypesEnd());
            }
            break;
        }
        default:
            return lyric_compiler::CompilerStatus::forCondition(lyric_compiler::CompilerCondition::kSyntaxError,
                "type {} cannot be used as the target of a match expression",
                targetType.getConcreteUrl().toString());
    }

    if (derive == lyric_object::DeriveType::Any) {
        // if target symbol is not sealed or final then we can't exhaustively check
        return patchList.size() == 1 && patchList.front().getPredicateType() == targetType;
    }

    // if target symbol is final then target set is empty so insert the target type as the only member
    if (derive == lyric_object::DeriveType::Final) {
        targetSet.insert(targetType);
    }

    // verify each member in the target set matches a predicate
    for (const auto &matchWhenPatch : patchList) {
        auto predicateType = matchWhenPatch.getPredicateType();
        auto iterator = targetSet.find(predicateType);
        if (iterator == targetSet.cend())
            return false;
        targetSet.erase(iterator);
    }

    // if target set is empty then the matching is exhaustive
    return targetSet.empty();
}

/**
 *
 * @param targetType
 * @param patchList
 * @return
 */
static tempo_utils::Result<bool>
check_placeholder_target_is_exhaustive(
    const lyric_common::TypeDef &targetType,
    const std::vector<lyric_assembler::MatchWhenPatch> &patchList,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
{
    TU_ASSERT (targetType.getType() == lyric_common::TypeDefType::Placeholder);
    auto *typeSystem = driver->getTypeSystem();

    std::pair<lyric_object::BoundType,lyric_common::TypeDef> bound;
    TU_ASSIGN_OR_RETURN (bound, typeSystem->resolveBound(targetType));
    if (bound.first != lyric_object::BoundType::None && bound.first != lyric_object::BoundType::Extends)
        return lyric_compiler::CompilerStatus::forCondition(lyric_compiler::CompilerCondition::kTypeError,
            "invalid target type {}; constraint must have Extends bounds",
            targetType.toString());

    return check_concrete_target_is_exhaustive(bound.second, patchList, block, driver);
}

/**
 *
 * @param targetType
 * @param patchList
 * @return
 */
static tempo_utils::Result<bool>
check_union_target_is_exhaustive(
    const lyric_common::TypeDef &targetType,
    const std::vector<lyric_assembler::MatchWhenPatch> &patchList,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
{
    for (const auto &memberType : targetType.getUnionMembers()) {
        bool isExhaustive;
        switch (memberType.getType()) {
            case lyric_common::TypeDefType::Concrete: {
                TU_ASSIGN_OR_RETURN (isExhaustive, check_concrete_target_is_exhaustive(
                    memberType, patchList, block, driver));
                break;
            }
            case lyric_common::TypeDefType::Placeholder: {
                TU_ASSIGN_OR_RETURN (isExhaustive, check_placeholder_target_is_exhaustive(
                    memberType, patchList, block, driver));
                break;
            }
            default:
                return lyric_compiler::CompilerStatus::forCondition(lyric_compiler::CompilerCondition::kTypeError,
                    "invalid target type member {}", memberType.toString());
        }
        if (!isExhaustive)
            return false;
    }

    // if every union member matches then matching is exhaustive
    return true;
}

tempo_utils::Status
lyric_compiler::check_match_is_exhaustive(
    Match *match,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
{
    const auto &targetType = match->targetRef.typeDef;
    bool isExhaustive = false;

    std::vector<lyric_assembler::MatchWhenPatch> patchList;
    for (auto &consequent : match->consequents) {
        patchList.push_back(consequent->patch);
    }

    // if there is no alternative clause, then determine whether the match is exhaustive by verifying
    // that all subtypes of the target are enumerable and there is a branch for all subtypes.
    switch (targetType.getType()) {
        case lyric_common::TypeDefType::Concrete: {
            TU_ASSIGN_OR_RETURN (isExhaustive, check_concrete_target_is_exhaustive(
                targetType, patchList, block, driver));
            break;
        }
        case lyric_common::TypeDefType::Placeholder: {
            TU_ASSIGN_OR_RETURN (isExhaustive, check_placeholder_target_is_exhaustive(
                targetType, patchList, block, driver));
            break;
        }
        case lyric_common::TypeDefType::Union: {
            TU_ASSIGN_OR_RETURN (isExhaustive, check_union_target_is_exhaustive(
                targetType, patchList, block, driver));
            break;
        }
        default:
            break;
    }

    if (!isExhaustive)
        return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
            "match expression has no else clause and target type {} cannot be checked exhaustively",
            targetType.toString());

    return {};
}
