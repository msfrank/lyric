
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/resolve_assignable.h>
#include <lyric_typing/resolve_pack.h>

tempo_utils::Result<lyric_assembler::ParameterPack>
lyric_typing::resolve_pack(
    const PackSpec &packSpec,
    lyric_assembler::AbstractResolver *resolver,
    lyric_assembler::AssemblyState *state)
{
    lyric_assembler::ParameterPack parameterPack;
    absl::flat_hash_set<std::string> names;
    absl::flat_hash_set<std::string> labels;
    int firstPosWithDefault = -1;

    auto *typeCache = state->typeCache();

    for (const auto &p : packSpec.listParameterSpec) {
        lyric_common::TypeDef paramType;
        TU_ASSIGN_OR_RETURN (paramType, resolve_assignable(p.type, resolver, state));

        lyric_assembler::Parameter param;
        param.index = parameterPack.listParameters.size();
        param.name = p.name;
        param.label = !p.label.empty()? p.label : p.name;
        param.placement = !p.init.isEmpty()? lyric_object::PlacementType::ListOpt : lyric_object::PlacementType::List;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE;
        param.typeDef = paramType;

        if (!p.init.isEmpty()) {
            if (firstPosWithDefault < 0) {
                firstPosWithDefault = param.index;
            }
        } else {
            if (firstPosWithDefault >= 0) {
                return state->logAndContinue(lyric_assembler::AssemblerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid list parameter {}; all list parameters after {} must be default-initialized",
                    p.name, parameterPack.listParameters.at(firstPosWithDefault).name);
            }
        }

        if (names.contains(p.name))
            return state->logAndContinue(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "invalid list parameter; parameter {} already defined", p.name);
        names.insert(p.name);

        if (labels.contains(param.label))
            return state->logAndContinue(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "invalid list parameter; label {} already defined", p.label);
        labels.insert(param.label);

        typeCache->touchType(param.typeDef);
        parameterPack.listParameters.push_back(param);
    }

    for (const auto &p : packSpec.namedParameterSpec) {
        lyric_common::TypeDef paramType;
        TU_ASSIGN_OR_RETURN (paramType, resolve_assignable(p.type, resolver, state));

        lyric_assembler::Parameter param;
        param.index = parameterPack.namedParameters.size();
        param.name = p.name;
        param.label = !p.label.empty()? p.label : p.name;
        param.placement = !p.init.isEmpty()? lyric_object::PlacementType::NamedOpt : lyric_object::PlacementType::Named;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE;
        param.typeDef = paramType;

        if (names.contains(p.name))
            return state->logAndContinue(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "invalid named parameter; parameter {} already defined", p.name);
        names.insert(p.name);

        if (labels.contains(param.label))
            return state->logAndContinue(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "invalid named parameter; label {} already defined", p.label);
        labels.insert(param.label);

        typeCache->touchType(param.typeDef);
        parameterPack.namedParameters.push_back(param);
    }

    for (const auto &p : packSpec.ctxParameterSpec) {
        lyric_common::TypeDef paramType;
        TU_ASSIGN_OR_RETURN (paramType, resolve_assignable(p.type, resolver, state));

        lyric_assembler::Parameter param;
        param.index = parameterPack.namedParameters.size();
        param.placement = lyric_object::PlacementType::Ctx;
        param.isVariable = false;
        param.typeDef = paramType;

        // if ctx parameter name is not specified, then generate a unique name
        param.name = p.name.empty()? absl::StrCat("$ctx", param.index) : p.name;
        param.label = param.name;

        if (names.contains(p.name))
            return state->logAndContinue(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "invalid ctx parameter; parameter {} already defined", p.name);
        names.insert(p.name);

        if (labels.contains(param.label))
            return state->logAndContinue(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "invalid ctx parameter; label {} already defined", p.label);
        labels.insert(param.label);

        typeCache->touchType(param.typeDef);
        parameterPack.namedParameters.push_back(param);
    }

    if (!packSpec.restParameterSpec.isEmpty()) {
        const auto &p = packSpec.restParameterSpec.getValue();

        lyric_common::TypeDef paramType;
        TU_ASSIGN_OR_RETURN (paramType, resolve_assignable(p.type, resolver, state));

        lyric_assembler::Parameter param;
        param.index = 0;
        param.name = p.name;
        param.label = param.name;
        param.placement = lyric_object::PlacementType::Rest;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = paramType;

        if (names.contains(p.name))
            return state->logAndContinue(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "invalid rest parameter; parameter {} already defined", p.name);
        names.insert(p.name);

        if (labels.contains(param.label))
            return state->logAndContinue(lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "invalid rest parameter; label {} already defined", p.label);
        labels.insert(param.label);

        typeCache->touchType(param.typeDef);
        parameterPack.restParameter = Option(param);
    }

    return parameterPack;
}
