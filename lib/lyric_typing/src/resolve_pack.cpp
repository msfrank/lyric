
#include <lyric_assembler/pack_builder.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/resolve_assignable.h>
#include <lyric_typing/resolve_pack.h>

tempo_utils::Result<lyric_assembler::ParameterPack>
lyric_typing::resolve_pack(
    const PackSpec &packSpec,
    lyric_assembler::AbstractResolver *resolver,
    lyric_assembler::AssemblyState *state)
{
    lyric_assembler::PackBuilder packBuilder;

    for (const auto &p : packSpec.listParameterSpec) {
        auto label = !p.label.empty() ? p.label : p.name;
        auto isVariable = p.binding == lyric_parser::BindingType::VARIABLE;
        lyric_common::TypeDef paramType;
        TU_ASSIGN_OR_RETURN (paramType, resolve_assignable(p.type, resolver, state));
        if (p.init.isEmpty()) {
            TU_RETURN_IF_NOT_OK (packBuilder.appendListParameter(p.name, label, paramType, isVariable));
        } else {
            TU_RETURN_IF_NOT_OK (packBuilder.appendListOptParameter(p.name, label, paramType, isVariable));
        }
    }

    for (const auto &p : packSpec.namedParameterSpec) {
        auto label = !p.label.empty() ? p.label : p.name;
        auto isVariable = p.binding == lyric_parser::BindingType::VARIABLE;
        lyric_common::TypeDef paramType;
        TU_ASSIGN_OR_RETURN (paramType, resolve_assignable(p.type, resolver, state));
        if (p.init.isEmpty()) {
            TU_RETURN_IF_NOT_OK (packBuilder.appendNamedParameter(p.name, label, paramType, isVariable));
        } else {
            TU_RETURN_IF_NOT_OK (packBuilder.appendNamedOptParameter(p.name, label, paramType, isVariable));
        }
    }

    for (const auto &p : packSpec.ctxParameterSpec) {
        auto label = !p.label.empty() ? p.label : p.name;
        lyric_common::TypeDef paramType;
        TU_ASSIGN_OR_RETURN (paramType, resolve_assignable(p.type, resolver, state));
        TU_RETURN_IF_NOT_OK (packBuilder.appendCtxParameter(p.name, label, paramType));
    }

    if (!packSpec.restParameterSpec.isEmpty()) {
        const auto &p = packSpec.restParameterSpec.peekValue();
        lyric_common::TypeDef paramType;
        TU_ASSIGN_OR_RETURN (paramType, resolve_assignable(p.type, resolver, state));
        TU_RETURN_IF_NOT_OK (packBuilder.appendRestParameter(p.name, paramType));
    }

    return packBuilder.toParameterPack();
}
