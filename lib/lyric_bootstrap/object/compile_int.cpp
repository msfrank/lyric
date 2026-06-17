
#include "compile_int.h"
#include "prelude_symbols.h"

CoreExistential *
declare_core_I64(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"I64"});
    auto *I64Existential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return I64Existential;
}

void build_core_I64(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *I32Type = preludeSymbols.I32Existential->existentialType;
    auto *I16Type = preludeSymbols.I16Existential->existentialType;
    auto *I8Type = preludeSymbols.I8Existential->existentialType;
    auto *I64Existential = preludeSymbols.I64Existential;
    auto *ConverterConcept = preludeSymbols.ConverterConcept;

    auto *I32ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {I32Type, I64Type});
    auto *I32ConverterImpl = state.addImpl(I64Existential->existentialPath, I32ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_I64));
        state.addImplExtension("Convert", I32ConverterImpl,
            {
                make_list_param("source", I32Type),
            },
            code, I64Type, true);
    }

    auto *I16ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {I16Type, I64Type});
    auto *I16ConverterImpl = state.addImpl(I64Existential->existentialPath, I16ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_I64));
        state.addImplExtension("Convert", I16ConverterImpl,
            {
                make_list_param("source", I16Type),
            },
            code, I64Type, true);
    }

    auto *I8ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {I8Type, I64Type});
    auto *I8ConverterImpl = state.addImpl(I64Existential->existentialPath, I8ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_I64));
        state.addImplExtension("Convert", I8ConverterImpl,
            {
                make_list_param("source", I8Type),
            },
            code, I64Type, true);
    }
}

CoreExistential *
declare_core_I32(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"I32"});
    auto *I32Existential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return I32Existential;
}

void build_core_I32(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *I32Type = preludeSymbols.I32Existential->existentialType;
    auto *I16Type = preludeSymbols.I16Existential->existentialType;
    auto *I8Type = preludeSymbols.I8Existential->existentialType;
    auto *I32Existential = preludeSymbols.I32Existential;
    auto *ConverterConcept = preludeSymbols.ConverterConcept;

    auto *I16ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {I16Type, I32Type});
    auto *I16ConverterImpl = state.addImpl(I32Existential->existentialPath, I16ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_I32));
        state.addImplExtension("Convert", I16ConverterImpl,
            {
                make_list_param("source", I16Type),
            },
            code, I32Type, true);
    }

    auto *I8ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {I8Type, I32Type});
    auto *I8ConverterImpl = state.addImpl(I32Existential->existentialPath, I8ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_I32));
        state.addImplExtension("Convert", I8ConverterImpl,
            {
                make_list_param("source", I8Type),
            },
            code, I32Type, true);
    }
}

CoreExistential *
declare_core_I16(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"I16"});
    auto *I16Existential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return I16Existential;
}

void build_core_I16(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *I16Type = preludeSymbols.I16Existential->existentialType;
    auto *I8Type = preludeSymbols.I8Existential->existentialType;
    auto *I16Existential = preludeSymbols.I16Existential;
    auto *ConverterConcept = preludeSymbols.ConverterConcept;

    auto *I8ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {I8Type, I16Type});
    auto *I8ConverterImpl = state.addImpl(I16Existential->existentialPath, I8ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_I16));
        state.addImplExtension("Convert", I8ConverterImpl,
            {
                make_list_param("source", I8Type),
            },
            code, I16Type, true);
    }
}

CoreExistential *
declare_core_I8(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"I8"});
    auto *I8Existential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return I8Existential;
}

CoreExistential *
declare_core_U64(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"U64"});
    auto *U64Existential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return U64Existential;
}

void build_core_U64(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *U64Type = preludeSymbols.U64Existential->existentialType;
    auto *U32Type = preludeSymbols.U32Existential->existentialType;
    auto *U16Type = preludeSymbols.U16Existential->existentialType;
    auto *U8Type = preludeSymbols.U8Existential->existentialType;
    auto *U64Existential = preludeSymbols.U64Existential;
    auto *ConverterConcept = preludeSymbols.ConverterConcept;

    auto *U32ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {U32Type, U64Type});
    auto *U32ConverterImpl = state.addImpl(U64Existential->existentialPath, U32ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_U64));
        state.addImplExtension("Convert", U32ConverterImpl,
            {
                make_list_param("source", U32Type),
            },
            code, U64Type, true);
    }

    auto *U16ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {U16Type, U64Type});
    auto *U16ConverterImpl = state.addImpl(U64Existential->existentialPath, U16ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_U64));
        state.addImplExtension("Convert", U16ConverterImpl,
            {
                make_list_param("source", U16Type),
            },
            code, U64Type, true);
    }

    auto *U8ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {U8Type, U64Type});
    auto *U8ConverterImpl = state.addImpl(U64Existential->existentialPath, U8ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_U64));
        state.addImplExtension("Convert", U8ConverterImpl,
            {
                make_list_param("source", U8Type),
            },
            code, U64Type, true);
    }
}

CoreExistential *
declare_core_U32(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"U32"});
    auto *U32Existential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return U32Existential;
}

void build_core_U32(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *U32Type = preludeSymbols.U32Existential->existentialType;
    auto *U16Type = preludeSymbols.U16Existential->existentialType;
    auto *U8Type = preludeSymbols.U8Existential->existentialType;
    auto *U32Existential = preludeSymbols.U32Existential;
    auto *ConverterConcept = preludeSymbols.ConverterConcept;

    auto *U16ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {U16Type, U32Type});
    auto *U16ConverterImpl = state.addImpl(U32Existential->existentialPath, U16ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_U32));
        state.addImplExtension("Convert", U16ConverterImpl,
            {
                make_list_param("source", U16Type),
            },
            code, U32Type, true);
    }

    auto *U8ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {U8Type, U32Type});
    auto *U8ConverterImpl = state.addImpl(U32Existential->existentialPath, U8ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_U32));
        state.addImplExtension("Convert", U8ConverterImpl,
            {
                make_list_param("source", U8Type),
            },
            code, U32Type, true);
    }
}

CoreExistential *
declare_core_U16(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"U16"});
    auto *U16Existential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return U16Existential;
}

void build_core_U16(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *U16Type = preludeSymbols.U16Existential->existentialType;
    auto *U8Type = preludeSymbols.U8Existential->existentialType;
    auto *U16Existential = preludeSymbols.U16Existential;
    auto *ConverterConcept = preludeSymbols.ConverterConcept;

    auto *U8ConverterType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, ConverterConcept->concept_index,
        {U8Type, U16Type});
    auto *U8ConverterImpl = state.addImpl(U16Existential->existentialPath, U8ConverterType, ConverterConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_TO_U16));
        state.addImplExtension("Convert", U8ConverterImpl,
            {
                make_list_param("source", U8Type),
            },
            code, U16Type, true);
    }
}

CoreExistential *
declare_core_U8(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"U8"});
    auto *U8Existential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return U8Existential;
}

CoreInstance *
build_core_IntInstance(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *BoolType = preludeSymbols.BoolExistential->existentialType;
    auto *I64Type = preludeSymbols.I64Existential->existentialType;

    lyric_common::SymbolPath instancePath({"IntInstance"});

    auto *IntArithmeticType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.ArithmeticConcept->concept_index,
        {I64Type});

    auto *IntComparisonType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.ComparisonConcept->concept_index,
        {I64Type, I64Type});

    auto *IntEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.EqualityConcept->concept_index,
        {I64Type, I64Type});

    auto *IntOrderedType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.OrderedConcept->concept_index,
        {I64Type});

    auto *IntInstance = state.addInstance(instancePath,
        lyo1::InstanceFlags::NONE, preludeSymbols.SingletonInstance);
    auto *IntArithmeticImpl = state.addImpl(instancePath, IntArithmeticType, preludeSymbols.ArithmeticConcept);
    auto *IntComparisonImpl = state.addImpl(instancePath, IntComparisonType, preludeSymbols.ComparisonConcept);
    auto *IntEqualityImpl = state.addImpl(instancePath, IntEqualityType, preludeSymbols.EqualityConcept);
    auto *IntOrderedImpl = state.addImpl(instancePath, IntOrderedType, preludeSymbols.OrderedConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_ADD));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Add", IntArithmeticImpl,
            {
                make_list_param("lhs", I64Type),
                make_list_param("rhs", I64Type),
            },
            code, I64Type, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_SUB));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Subtract", IntArithmeticImpl,
            {
                make_list_param("lhs", I64Type),
                make_list_param("rhs", I64Type),
            },
            code, I64Type, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_MUL));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Multiply", IntArithmeticImpl,
            {
                make_list_param("lhs", I64Type),
                make_list_param("rhs", I64Type),
            },
            code, I64Type, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_DIV));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Divide", IntArithmeticImpl,
            {
                make_list_param("lhs", I64Type),
                make_list_param("rhs", I64Type),
            },
            code, I64Type, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NEG));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Negate", IntArithmeticImpl,
            {
                make_list_param("lhs", I64Type),
            },
            code, I64Type, true);
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
        state.addImplExtension("Equals", IntEqualityImpl,
            {
                make_list_param("lhs", I64Type),
                make_list_param("rhs", I64Type),
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
        state.addImplExtension("LessThan", IntComparisonImpl,
            {
                make_list_param("lhs", I64Type),
                make_list_param("rhs", I64Type),
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
        state.addImplExtension("GreaterThan", IntComparisonImpl,
            {
                make_list_param("lhs", I64Type),
                make_list_param("rhs", I64Type),
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
        state.addImplExtension("LessEquals", IntComparisonImpl,
            {
                make_list_param("lhs", I64Type),
                make_list_param("rhs", I64Type),
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
        state.addImplExtension("GreaterEquals", IntComparisonImpl,
            {
                make_list_param("lhs", I64Type),
                make_list_param("rhs", I64Type),
            },
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.loadArgument(0));
        TU_RAISE_IF_NOT_OK(code.loadArgument(1));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_CMP));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Compare", IntOrderedImpl,
            {
                make_list_param("lhs", I64Type),
                make_list_param("rhs", I64Type),
            },
            code, I64Type, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(IntInstance, {}, code);
        state.setInstanceAllocator(IntInstance, "SingletonAlloc");
    }

    return IntInstance;
}
