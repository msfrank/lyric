
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_typing/check_dispatchable.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/typing_result.h>

tempo_utils::Status
lyric_typing::check_dispatchable(
    lyric_assembler::AbstractSymbol *symbol,
    const lyric_assembler::ParameterPack &parameterPack,
    const lyric_common::TypeDef &returnType,
    lyric_assembler::ObjectState *state)
{
    TU_NOTNULL (symbol);
    TU_NOTNULL (state);

    std::vector<lyric_assembler::Parameter> listParameters;
    std::vector<lyric_assembler::Parameter> namedParameters;
    std::optional<lyric_assembler::Parameter> restParameter;
    lyric_common::TypeDef returns;

    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::ACTION: {
            auto *actionSymbol = lyric_assembler::cast_symbol_to_action(symbol);
            listParameters.insert(listParameters.cbegin(),
                actionSymbol->listPlacementBegin(), actionSymbol->listPlacementEnd());
            namedParameters.insert(namedParameters.cbegin(),
                actionSymbol->namedPlacementBegin(), actionSymbol->namedPlacementEnd());
            auto *rest = actionSymbol->restPlacement();
            if (rest != nullptr) {
                restParameter = *rest;
            }
            returns = actionSymbol->getReturnType();
            break;
        }
        case lyric_assembler::SymbolType::CALL: {
            auto *callSymbol = lyric_assembler::cast_symbol_to_call(symbol);
            listParameters.insert(listParameters.cbegin(),
                callSymbol->listPlacementBegin(), callSymbol->listPlacementEnd());
            namedParameters.insert(namedParameters.cbegin(),
                callSymbol->namedPlacementBegin(), callSymbol->namedPlacementEnd());
            auto *rest = callSymbol->restPlacement();
            if (rest != nullptr) {
                restParameter = *rest;
            }
            returns = callSymbol->getReturnType();
            break;
        }
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "{} is not dispatchable", symbol->getSymbolUrl().toString());
    }

    // check total number of parameters
    auto numExpectedParams = listParameters.size() + namedParameters.size()
        + (restParameter.has_value()? 1 : 0);
    auto numActualParams = parameterPack.listParameters.size() + parameterPack.namedParameters.size()
        + (parameterPack.restParameter.hasValue()? 1 : 0);
    if (numActualParams != numExpectedParams)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "wrong number of parameters; expected {} but found {}", numExpectedParams, numActualParams);

    // check number of list parameters
    if (listParameters.size() != parameterPack.listParameters.size())
        return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
            "wrong number of list parameters; expected {} but found {}",
            parameterPack.listParameters.size(), listParameters.size());

    // check number of named parameters
    if (namedParameters.size() != parameterPack.namedParameters.size())
        return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
            "wrong number of named parameters; expected {} but found {}",
            parameterPack.namedParameters.size(), namedParameters.size());

    /* NOTE: we don't need to check for presence of rest parameter because it will be detected
     * implicitly by either the total param count check or the combination of list and named param
     * count checks.
     */

    // check that each list param matches type and placement
    for (size_t i = 0; i < listParameters.size(); i++) {
        const auto &actual = parameterPack.listParameters.at(i);
        const auto &expected = listParameters.at(i);

        if (actual.placement != expected.placement)
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "'{}' parameter is incompatible with {}",
                actual.name, symbol->getSymbolUrl().toString());

        lyric_runtime::TypeComparison cmp;
        TU_ASSIGN_OR_RETURN (cmp, compare_assignable(actual.typeDef, expected.typeDef, state));
        switch (cmp) {
            case lyric_runtime::TypeComparison::EQUAL:
                break;
            default:
                return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                    "'{}' parameter is incompatible with {}",
                    actual.name, symbol->getSymbolUrl().toString());
        }
    }

    // check that each named param matches name, type, and placement
    for (size_t i = 0; i < namedParameters.size(); i++) {
        const auto &actual = parameterPack.namedParameters.at(i);
        const auto &expected = namedParameters.at(i);

        if (actual.name != expected.name)
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "'{}' parameter is incompatible with {}",
                actual.name, symbol->getSymbolUrl().toString());

        if (actual.placement != expected.placement)
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "'{}' parameter is incompatible with {}",
                actual.name, symbol->getSymbolUrl().toString());

        lyric_runtime::TypeComparison cmp;
        TU_ASSIGN_OR_RETURN (cmp, compare_assignable(actual.typeDef, expected.typeDef, state));
        switch (cmp) {
            case lyric_runtime::TypeComparison::EQUAL:
                break;
            default:
                return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                    "'{}' parameter is incompatible with {}",
                    actual.name, symbol->getSymbolUrl().toString());
        }
    }

    // check that rest param matches type and placement
    if (restParameter.has_value()) {
        const auto &actual = parameterPack.restParameter.peekValue();
        const auto &expected = restParameter.value();

        if (actual.placement != expected.placement)
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "rest' parameter is incompatible with {}", symbol->getSymbolUrl().toString());

        lyric_runtime::TypeComparison cmp;
        TU_ASSIGN_OR_RETURN (cmp, compare_assignable(actual.typeDef, expected.typeDef, state));
        switch (cmp) {
            case lyric_runtime::TypeComparison::EQUAL:
                break;
            default:
                return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                    "rest parameter is incompatible with {}", symbol->getSymbolUrl().toString());
        }
    }

    // check return type

    return {};
}
