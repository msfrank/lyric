
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_typing/check_dispatchable.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/typing_result.h>

inline std::string placement_type_to_string(lyric_object::PlacementType placement)
{
    switch (placement) {
        case lyric_object::PlacementType::List:     return "list";
        case lyric_object::PlacementType::ListOpt:  return "list-optional";
        case lyric_object::PlacementType::Named:    return "named";
        case lyric_object::PlacementType::NamedOpt: return "named-optional";
        case lyric_object::PlacementType::Ctx:      return "using";
        case lyric_object::PlacementType::Rest:     return "rest";
        default:                                    return "???";
    }
}

tempo_utils::Status
lyric_typing::check_dispatchable(
    const lyric_assembler::ParameterPack &toParameters,
    const lyric_common::TypeDef &toResult,
    const lyric_assembler::ParameterPack &fromParameters,
    const lyric_common::TypeDef &fromResult,
    lyric_assembler::ObjectState *state)
{
    TU_NOTNULL (state);

    auto unifiedToParameters = toParameters.getUnifiedParameters();
    auto unifiedFromParameters = fromParameters.getUnifiedParameters();

    // check total number of parameters
    if (unifiedToParameters.size() != unifiedFromParameters.size())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "wrong number of parameters; expected {} but found {}",
            unifiedToParameters.size(), unifiedFromParameters.size());

    // check each parameter
    for (size_t i = 0; i < unifiedToParameters.size(); i++) {
        const auto &fromParam = unifiedFromParameters.at(i);
        const auto &toParam = unifiedToParameters.at(i);

        // placement types must match
        if (fromParam.placement != toParam.placement)
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "'{}' parameter is incompatible with base call; expected a {} parameter",
                fromParam.name, placement_type_to_string(toParam.placement));

        // if parameter is named, then the parameter names must match
        switch (fromParam.placement) {
            case lyric_object::PlacementType::Named:
            case lyric_object::PlacementType::NamedOpt:
            case lyric_object::PlacementType::Ctx:
                if (fromParam.name != toParam.name)
                    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                        "named parameter '{}' must match name '{}' from base call",
                        fromParam.name, toParam.name);
            default:
                break;
        }

        // parameter types must match
        lyric_runtime::TypeComparison cmp;
        TU_ASSIGN_OR_RETURN (cmp, compare_assignable(fromParam.typeDef, toParam.typeDef, state));
        switch (cmp) {
            case lyric_runtime::TypeComparison::EQUAL:
                break;
            default:
                return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                    "'{}' parameter is incompatible with type {} from base call",
                    fromParam.name, toParam.typeDef.toString());
        }
    }

    // check return type
    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, compare_assignable(fromResult, toResult, state));
    switch (cmp) {
        case lyric_runtime::TypeComparison::EQUAL:
            break;
        default:
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "return type is incompatible with {} from base call", toResult.toString());
    }

    return {};
}
