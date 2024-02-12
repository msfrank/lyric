
#include <lyric_common/symbol_url.h>

#include "compile_float.h"

CoreExistential *
build_core_Float(BuilderState &state, const CoreExistential *IntrinsicExistential)
{
    lyric_common::SymbolPath existentialPath({"Float"});
    auto *FloatExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Float64,
        lyo1::ExistentialFlags::Final, IntrinsicExistential);
    return FloatExistential;
}

CoreInstance *
build_core_FloatInstance(
    BuilderState &state,
    const CoreType *FloatType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ArithmeticConcept,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *IntegerType,
    const CoreType *BoolType)
{
    lyric_common::SymbolPath instancePath({"FloatInstance"});

    auto *FloatArithmeticType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        ArithmeticConcept->concept_index,
        {FloatType, FloatType});

    auto *FloatComparisonType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        ComparisonConcept->concept_index,
        {FloatType, FloatType});

    auto *FloatEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        EqualityConcept->concept_index,
        {FloatType, FloatType});

    auto *FloatOrderedType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        OrderedConcept->concept_index,
        {FloatType});

    auto *FloatInstance = state.addInstance(instancePath, lyo1::InstanceFlags::NONE, SingletonInstance);

    auto *FloatArithmeticImpl = state.addImpl(instancePath, FloatArithmeticType, ArithmeticConcept);
    auto *FloatComparisonImpl = state.addImpl(instancePath, FloatComparisonType, ComparisonConcept);
    auto *FloatEqualityImpl = state.addImpl(instancePath, FloatEqualityType, EqualityConcept);
    auto *FloatOrderedImpl = state.addImpl(instancePath, FloatOrderedType, OrderedConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_ADD));
        state.addImplExtension("add", FloatArithmeticImpl,
            {{"lhs", FloatType}, {"rhs", FloatType}}, {},
            code, FloatType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_SUB));
        state.addImplExtension("subtract", FloatArithmeticImpl,
            {{"lhs", FloatType}, {"rhs", FloatType}}, {},
            code, FloatType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_MUL));
        state.addImplExtension("multiply", FloatArithmeticImpl,
            {{"lhs", FloatType}, {"rhs", FloatType}}, {},
            code, FloatType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_DIV));
        state.addImplExtension("divide", FloatArithmeticImpl,
            {{"lhs", FloatType}, {"rhs", FloatType}}, {},
            code, FloatType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_NEG));
        state.addImplExtension("negate", FloatArithmeticImpl,
            {{"lhs", FloatType}}, {},
            code, FloatType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_CMP));
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
        state.addImplExtension("equals", FloatEqualityImpl,
            {{"lhs", FloatType}, {"rhs", FloatType}}, {},
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_CMP));
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
        state.addImplExtension("lessthan", FloatComparisonImpl,
            {{"lhs", FloatType}, {"rhs", FloatType}}, {},
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_CMP));
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
        state.addImplExtension("greaterthan", FloatComparisonImpl,
            {{"lhs", FloatType}, {"rhs", FloatType}}, {},
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_CMP));
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
        state.addImplExtension("lessequals", FloatComparisonImpl,
            {{"lhs", FloatType}, {"rhs", FloatType}}, {},
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_CMP));
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
        state.addImplExtension("greaterequals", FloatComparisonImpl,
            {{"lhs", FloatType}, {"rhs", FloatType}}, {},
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DBL_CMP));
        state.addImplExtension("compare", FloatOrderedImpl,
            {{"lhs", FloatType}, {"rhs", FloatType}}, {},
            code, IntegerType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(FloatInstance, {}, code);
        state.setInstanceAllocator(FloatInstance, lyric_bootstrap::internal::BootstrapTrap::SINGLETON_ALLOC);
    }

    return FloatInstance;
}