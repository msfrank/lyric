
#include <lyric_common/symbol_url.h>

#include "compile_type.h"

CoreExistential *
declare_core_Type(BuilderState &state, const CoreExistential *AnyExistential)
{
    lyric_common::SymbolPath existentialPath({"Type"});
    auto *TypeExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Invalid,
        lyo1::ExistentialFlags::Final, AnyExistential);
    return TypeExistential;
}

void
build_core_Type(
    BuilderState &state,
    const CoreExistential *TypeExistential,
    const CoreType *IntegerType,
    const CoreType *BoolType)
{
    auto *TypeType = TypeExistential->existentialType;

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadReceiver());
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_TYPE_CMP));
        state.addExistentialMethod("Compare", TypeExistential,
            {},
            {
                make_list_param("other", TypeType),
            },
            code,
            IntegerType,
            /* isInline= */ true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadReceiver());
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_TYPE_CMP));
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.jumpIfGreaterThan(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
        state.addExistentialMethod("IsSupertypeOf", TypeExistential,
            {},
            {
                make_list_param("other", TypeType),
            },
            code,
            BoolType,
            /* isInline= */ true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadReceiver());
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_TYPE_CMP));
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.jumpIfLessThan(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
        state.addExistentialMethod("IsSubtypeOf", TypeExistential,
            {},
            {
                make_list_param("other", TypeType),
            },
            code,
            BoolType,
            /* isInline= */ true);
    }
}