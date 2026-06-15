
#include "compile_int.h"

CoreExistential *
declare_core_Num(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Num"});
    auto *NumExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return NumExistential;
}

void
build_core_Num(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *NumType = preludeSymbols.NumExistential->existentialType;
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *I32Type = preludeSymbols.I32Existential->existentialType;
    auto *I16Type = preludeSymbols.I16Existential->existentialType;
    auto *I8Type = preludeSymbols.I8Existential->existentialType;
    auto *NumExistential = preludeSymbols.NumExistential;
    auto *ConverterConcept = preludeSymbols.ConverterConcept;

    auto *I64ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {NumType, I64Type});
    auto *I64ConverterImpl = state.addImpl(NumExistential->existentialPath, I64ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_I64));
        state.addImplExtension("Convert", I64ConverterImpl,
            {
                make_list_param("source", I64Type),
            },
            code, NumType, true);
    }

    auto *I32ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {NumType, I32Type});
    auto *I32ConverterImpl = state.addImpl(NumExistential->existentialPath, I32ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_I64));
        state.addImplExtension("Convert", I32ConverterImpl,
            {
                make_list_param("source", I32Type),
            },
            code, NumType, true);
    }

    auto *I16ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {NumType, I16Type});
    auto *I16ConverterImpl = state.addImpl(NumExistential->existentialPath, I16ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_I64));
        state.addImplExtension("Convert", I16ConverterImpl,
            {
                make_list_param("source", I16Type),
            },
            code, NumType, true);
    }

    auto *I8ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {NumType, I8Type});
    auto *I8ConverterImpl = state.addImpl(NumExistential->existentialPath, I8ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_I64));
        state.addImplExtension("Convert", I8ConverterImpl,
            {
                make_list_param("source", I8Type),
            },
            code, NumType, true);
    }
}

CoreInstance *
build_core_NumInstance(
    BuilderState &state,
    const CoreType *NumType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ArithmeticConcept,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *BoolType)
{
    lyric_common::SymbolPath instancePath({"NumInstance"});

    auto *NumArithmeticType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        ArithmeticConcept->concept_index,
        {NumType});

    auto *NumComparisonType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        ComparisonConcept->concept_index,
        {NumType, NumType});

    auto *NumEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        EqualityConcept->concept_index,
        {NumType, NumType});

    auto *NumOrderedType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        OrderedConcept->concept_index,
        {NumType});

    auto *NumInstance = state.addInstance(instancePath,
        lyo1::InstanceFlags::NONE, SingletonInstance);
    auto *NumArithmeticImpl = state.addImpl(instancePath, NumArithmeticType, ArithmeticConcept);
    auto *NumComparisonImpl = state.addImpl(instancePath, NumComparisonType, ComparisonConcept);
    auto *NumEqualityImpl = state.addImpl(instancePath, NumEqualityType, EqualityConcept);
    auto *NumOrderedImpl = state.addImpl(instancePath, NumOrderedType, OrderedConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_ADD));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Add", NumArithmeticImpl,
            {
                make_list_param("lhs", NumType),
                make_list_param("rhs", NumType),
            },
            code, NumType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_SUB));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Subtract", NumArithmeticImpl,
            {
                make_list_param("lhs", NumType),
                make_list_param("rhs", NumType),
            },
            code, NumType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_MUL));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Multiply", NumArithmeticImpl,
            {
                make_list_param("lhs", NumType),
                make_list_param("rhs", NumType),
            },
            code, NumType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DIV));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Divide", NumArithmeticImpl,
            {
                make_list_param("lhs", NumType),
                make_list_param("rhs", NumType),
            },
            code, NumType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NEG));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Negate", NumArithmeticImpl,
            {
                make_list_param("lhs", NumType),
            },
            code, NumType, true);
    }
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
        state.addImplExtension("Equals", NumEqualityImpl,
            {
                make_list_param("lhs", NumType),
                make_list_param("rhs", NumType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_CMP));
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.jumpIfLessThan(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("LessThan", NumComparisonImpl,
            {
                make_list_param("lhs", NumType),
                make_list_param("rhs", NumType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_CMP));
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.jumpIfGreaterThan(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("GreaterThan", NumComparisonImpl,
            {
                make_list_param("lhs", NumType),
                make_list_param("rhs", NumType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_CMP));
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.jumpIfLessOrEqual(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("LessEquals", NumComparisonImpl,
            {
                make_list_param("lhs", NumType),
                make_list_param("rhs", NumType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_CMP));
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.jumpIfGreaterOrEqual(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("GreaterEquals", NumComparisonImpl,
            {
                make_list_param("lhs", NumType),
                make_list_param("rhs", NumType),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_CMP));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Compare", NumOrderedImpl,
            {
                make_list_param("lhs", NumType),
                make_list_param("rhs", NumType),
            },
            code, NumType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(NumInstance, {}, code);
        state.setInstanceAllocator(NumInstance, "SingletonAlloc");
    }

    return NumInstance;
}