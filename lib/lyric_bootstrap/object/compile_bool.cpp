
#include "builder_state.h"
#include "compile_bool.h"

CoreExistential *
declare_core_Bool(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Bool"});
    auto *BoolExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return BoolExistential;
}

void
build_core_Bool(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
}

CoreInstance *
build_core_BoolInstance(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath instancePath({"BoolInstance"});

    auto *BoolType = preludeSymbols.BoolExistential->existentialType;
    auto *I64Type = preludeSymbols.I64Existential->existentialType;

    auto *BoolEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.EqualityConcept->concept_index,
        {BoolType, BoolType});

    auto *BoolOrderedType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.OrderedConcept->concept_index,
        {BoolType});

    auto *BoolPropositionType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.PropositionConcept->concept_index,
        {BoolType});

    auto *BoolInstance = state.addInstance(instancePath,
        lyo1::InstanceFlags::NONE, preludeSymbols.SingletonInstance);
    auto *BoolEqualityImpl = state.addImpl(instancePath, BoolEqualityType, preludeSymbols.EqualityConcept);
    auto *BoolOrderedImpl = state.addImpl(instancePath, BoolOrderedType, preludeSymbols.OrderedConcept);
    auto *BoolPropositionImpl = state.addImpl(instancePath, BoolPropositionType, preludeSymbols.PropositionConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_CMP));
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.jumpIfZero(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Equals", BoolEqualityImpl,
            {
                make_list_param("lhs", BoolType),
                make_list_param("rhs", BoolType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_CMP));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Compare", BoolOrderedImpl,
            {
                make_list_param("lhs", BoolType),
                make_list_param("rhs", BoolType),
            },
            code, I64Type, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_LOGICAL_AND));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Conjunct", BoolPropositionImpl,
            {
                make_list_param("lhs", BoolType),
                make_list_param("rhs", BoolType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_LOGICAL_OR));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Disjunct", BoolPropositionImpl,
            {
                make_list_param("lhs", BoolType),
                make_list_param("rhs", BoolType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_LOGICAL_NOT));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Complement", BoolPropositionImpl,
            {
                make_list_param("lhs", BoolType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(BoolInstance, {}, code);
        state.setInstanceAllocator(BoolInstance, "SingletonAlloc");
    }

    return BoolInstance;
}
