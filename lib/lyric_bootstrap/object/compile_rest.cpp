
#include "compile_rest.h"

CoreExistential *
declare_core_Rest(BuilderState &state, const CoreExistential *AnyExistential)
{
    lyric_common::SymbolPath existentialPath({"Rest"});

    std::vector<CorePlaceholder> placeholders;
    placeholders.push_back({"T", lyo1::PlaceholderVariance::Invariant});
    auto *RestTemplate = state.addTemplate(existentialPath, placeholders);

    auto *RestExistential = state.addGenericExistential(existentialPath, RestTemplate,
        lyo1::IntrinsicType::Invalid, lyo1::ExistentialFlags::Final, AnyExistential);
    return RestExistential;
}

void
build_core_Rest(
    BuilderState &state,
    const CoreExistential *RestExistential,
    const CoreType *IntType,
    const CoreType *UndefType)
{
    auto *TType = RestExistential->existentialTemplate->types.at("T");
    auto *TOrUndefType = state.addUnionType({TType,UndefType});

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "RestNumArgs");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("NumArgs",
            RestExistential,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code, IntType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "RestGetArg");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("GetArg",
            RestExistential,
            lyo1::CallFlags::GlobalVisibility,
            {
                make_list_param("index", IntType),
            },
            code, TOrUndefType);
    }
}

