
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/typing_result.h>
#include <lyric_typing/validate_subtype.h>

static tempo_utils::Status
validate_concrete_argument(
    const lyric_common::TypeDef &argType,
    const lyric_object::TemplateParameter &tp,
    lyric_assembler::ObjectState *state)
{
    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, lyric_typing::compare_assignable(tp.typeDef, argType, state));
    switch (cmp) {
        case lyric_runtime::TypeComparison::EQUAL:
            return {};
        case lyric_runtime::TypeComparison::EXTENDS:
            if (tp.bound == lyric_object::BoundType::Extends)
                return {};
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "type argument {} is incompatible", argType.toString());
        case lyric_runtime::TypeComparison::SUPER:
            if (tp.bound == lyric_object::BoundType::Super)
                return {};
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "type argument {} is incompatible", argType.toString());
        default:
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "type argument {} is incompatible", argType.toString());
    }
}

static tempo_utils::Status
validate_placeholder_argument(
    const lyric_common::TypeDef &argType,
    const lyric_object::TemplateParameter &tp,
    lyric_assembler::ObjectState *state)
{
    auto *typeCache = state->typeCache();
    auto *templateHandle = typeCache->getTemplate(argType.getPlaceholderTemplateUrl());
    auto argParameter = templateHandle->getTemplateParameter(argType.getPlaceholderIndex());

    if (argParameter.bound != tp.bound)
        return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
            "type argument {} is incompatible", argType.toString());

    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, lyric_typing::compare_assignable(tp.typeDef, argParameter.typeDef, state));
    switch (cmp) {
        case lyric_runtime::TypeComparison::EQUAL:
            return {};
        case lyric_runtime::TypeComparison::EXTENDS:
            if (tp.bound == lyric_object::BoundType::Extends)
                return {};
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "type argument {} is incompatible", argType.toString());
        case lyric_runtime::TypeComparison::SUPER:
            if (tp.bound == lyric_object::BoundType::Super)
                return {};
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "type argument {} is incompatible", argType.toString());
        default:
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "type argument {} is incompatible", argType.toString());
    }
}

static tempo_utils::Status
validate_union_argument(
    const lyric_common::TypeDef &argType,
    const lyric_object::TemplateParameter &tp,
    lyric_assembler::ObjectState *state)
{
    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, lyric_typing::compare_assignable(tp.typeDef, argType, state));
    switch (cmp) {
        case lyric_runtime::TypeComparison::EQUAL:
            return {};
        case lyric_runtime::TypeComparison::EXTENDS:
            if (tp.bound == lyric_object::BoundType::Extends)
                return {};
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "type argument {} is incompatible", argType.toString());
        case lyric_runtime::TypeComparison::SUPER:
            if (tp.bound == lyric_object::BoundType::Super)
                return {};
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "type argument {} is incompatible", argType.toString());
        default:
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
                "type argument {} is incompatible", argType.toString());
    }
}

static tempo_utils::Status
validate_intersection_argument(
    const lyric_common::TypeDef &argType,
    const lyric_object::TemplateParameter &tp,
    lyric_assembler::ObjectState *state)
{
    return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kIncompatibleType,
        "type argument {} is incompatible", argType.toString());
}

tempo_utils::Status
lyric_typing::validate_subtype(
    const lyric_common::TypeDef &subType,
    lyric_assembler::AbstractSymbol *symbol,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (subType.isValid());
    TU_NOTNULL (symbol);

    if (subType.getType() != lyric_common::TypeDefType::Concrete)
        return TypingStatus::forCondition(TypingCondition::kTypeError,
            "invalid subtype {}", subType.toString());

    lyric_assembler::TemplateHandle *templateHandle;
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS:
            templateHandle = lyric_assembler::cast_symbol_to_class(symbol)->classTemplate();
            break;
        case lyric_assembler::SymbolType::CONCEPT:
            templateHandle = lyric_assembler::cast_symbol_to_concept(symbol)->conceptTemplate();
            break;
        case lyric_assembler::SymbolType::EXISTENTIAL:
            templateHandle = lyric_assembler::cast_symbol_to_existential(symbol)->existentialTemplate();
            break;

        case lyric_assembler::SymbolType::ENUM:
        case lyric_assembler::SymbolType::INSTANCE:
        case lyric_assembler::SymbolType::STRUCT:
            if (subType.numConcreteArguments() > 0)
                return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                    "unexpected type arguments for subtype {} of {}",
                    subType.toString(), symbol->getSymbolUrl().toString());
            return {};

        default:
            return TypingStatus::forCondition(TypingCondition::kTypeError,
                "cannot subtype {}", symbol->getSymbolUrl().toString());
    }

    std::vector<lyric_object::TemplateParameter> templateParameters;
    if (templateHandle != nullptr) {
        templateParameters = templateHandle->getTemplateParameters();
    }

    if (subType.numConcreteArguments() != templateParameters.size())
        return TypingStatus::forCondition(TypingCondition::kTypeError,
            "wrong number of type arguments for {}; expected {} but found {}",
            subType.getConcreteUrl().toString(), templateParameters.size(), subType.numConcreteArguments());
    auto subtypeArguments = subType.getConcreteArguments();

    for (int i = 0; i < templateParameters.size(); i++) {
        const auto &tp = templateParameters.at(i);
        const auto &argType = subtypeArguments[i];
        switch (argType.getType()) {
            case lyric_common::TypeDefType::Concrete:
                TU_RETURN_IF_NOT_OK (validate_concrete_argument(argType, tp, state));
                break;
            case lyric_common::TypeDefType::Placeholder:
                TU_RETURN_IF_NOT_OK (validate_placeholder_argument(argType, tp, state));
                break;
            case lyric_common::TypeDefType::Union:
                TU_RETURN_IF_NOT_OK (validate_union_argument(argType, tp, state));
                break;
            case lyric_common::TypeDefType::Intersection:
                TU_RETURN_IF_NOT_OK (validate_intersection_argument(argType, tp, state));
                break;
            default:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "invalid type argument {}", argType.toString());
        }
    }

    return {};
}