
#include "compile_int.h"

CoreExistential *
declare_core_Int(BuilderState &state, const CoreExistential *IntrinsicExistential)
{
    lyric_common::SymbolPath existentialPath({"Int"});
    auto *IntExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Int64,
        lyo1::ExistentialFlags::Final, IntrinsicExistential);
    return IntExistential;
}

void
build_core_Int(BuilderState &state, const CoreExistential *IntExistential)
{
}

CoreInstance *
build_core_IntInstance(
    BuilderState &state,
    const CoreType *IntType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ArithmeticConcept,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *BoolType)
{
    lyric_common::SymbolPath instancePath({"IntInstance"});

    auto *IntArithmeticType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        ArithmeticConcept->concept_index,
        {IntType, IntType});

    auto *IntComparisonType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        ComparisonConcept->concept_index,
        {IntType, IntType});

    auto *IntEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        EqualityConcept->concept_index,
        {IntType, IntType});

    auto *IntOrderedType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        OrderedConcept->concept_index,
        {IntType});

    auto *IntInstance = state.addInstance(instancePath,
        lyo1::InstanceFlags::NONE, SingletonInstance);
    auto *IntArithmeticImpl = state.addImpl(instancePath, IntArithmeticType, ArithmeticConcept);
    auto *IntComparisonImpl = state.addImpl(instancePath, IntComparisonType, ComparisonConcept);
    auto *IntEqualityImpl = state.addImpl(instancePath, IntEqualityType, EqualityConcept);
    auto *IntOrderedImpl = state.addImpl(instancePath, IntOrderedType, OrderedConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_ADD));
        state.addImplExtension("add", IntArithmeticImpl,
            {
                make_list_param("lhs", IntType),
                make_list_param("rhs", IntType),
            },
            code, IntType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_SUB));
        state.addImplExtension("subtract", IntArithmeticImpl,
            {
                make_list_param("lhs", IntType),
                make_list_param("rhs", IntType),
            },
            code, IntType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_MUL));
        state.addImplExtension("multiply", IntArithmeticImpl,
            {
                make_list_param("lhs", IntType),
                make_list_param("rhs", IntType),
            },
            code, IntType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_DIV));
        state.addImplExtension("divide", IntArithmeticImpl,
            {
                make_list_param("lhs", IntType),
                make_list_param("rhs", IntType),
            },
            code, IntType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_NEG));
        state.addImplExtension("negate", IntArithmeticImpl,
            {
                make_list_param("lhs", IntType),
            },
            code, IntType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_CMP));
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.jumpIfZero(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
        state.addImplExtension("equals", IntEqualityImpl,
            {
                make_list_param("lhs", IntType),
                make_list_param("rhs", IntType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_CMP));
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
        state.addImplExtension("lessthan", IntComparisonImpl,
            {
                make_list_param("lhs", IntType),
                make_list_param("rhs", IntType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_CMP));
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
        state.addImplExtension("greaterthan", IntComparisonImpl,
            {
                make_list_param("lhs", IntType),
                make_list_param("rhs", IntType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_CMP));
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.jumpIfLessOrEqual(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
        state.addImplExtension("lessequals", IntComparisonImpl,
            {
                make_list_param("lhs", IntType),
                make_list_param("rhs", IntType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_CMP));
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.jumpIfGreaterOrEqual(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
        state.addImplExtension("greaterequals", IntComparisonImpl,
            {
                make_list_param("lhs", IntType),
                make_list_param("rhs", IntType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_I64_CMP));
        state.addImplExtension("compare", IntOrderedImpl,
            {
                make_list_param("lhs", IntType),
                make_list_param("rhs", IntType),
            },
            code, IntType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(IntInstance, {}, code);
        state.setInstanceAllocator(IntInstance, lyric_bootstrap::internal::BootstrapTrap::SINGLETON_ALLOC);
    }

    return IntInstance;
}
