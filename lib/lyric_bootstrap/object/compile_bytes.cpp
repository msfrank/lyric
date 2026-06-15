
#include "compile_bytes.h"
#include "prelude_symbols.h"

CoreExistential *
declare_core_Bytes(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Bytes"});
    auto *BytesExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return BytesExistential;
}

void
build_core_Bytes(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *StringType = preludeSymbols.StringExistential->existentialType;
    auto *UndefType = preludeSymbols.UndefExistential->existentialType;
    auto *BytesType = preludeSymbols.BytesExistential->existentialType;

    auto *IntOrUndefType = state.addUnionType({I64Type,UndefType});
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesLength");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Length",
            preludeSymbols.BytesExistential,
            lyo1::CallFlags::NONE,
            {},
            code, I64Type);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesAt");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("At",
            preludeSymbols.BytesExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", I64Type),
            },
            code, IntOrUndefType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesToString");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("ToString",
            preludeSymbols.BytesExistential,
            lyo1::CallFlags::NONE,
            {},
            code, StringType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesAppend");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Append",
            preludeSymbols.BytesExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("other", BytesType),
            },
            code, BytesType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesPrepend");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Prepend",
            preludeSymbols.BytesExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("other", BytesType),
            },
            code, BytesType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesInsert");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Insert",
            preludeSymbols.BytesExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", I64Type),
                make_list_param("other", BytesType),
            },
            code, BytesType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesRemove");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Remove",
            preludeSymbols.BytesExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", I64Type),
                make_list_param("count", I64Type),
            },
            code, BytesType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesSubspan");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Subspan",
            preludeSymbols.BytesExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", I64Type),
                make_list_param("count", I64Type),
            },
            code, BytesType);
    }
}

CoreInstance *
build_core_BytesInstance(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *BoolType = preludeSymbols.BoolExistential->existentialType;
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *BytesType = preludeSymbols.BytesExistential->existentialType;

    lyric_common::SymbolPath instancePath({"BytesInstance"});

    auto *BytesComparisonType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.ComparisonConcept->concept_index,
        {BytesType, BytesType});

    auto *BytesEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.EqualityConcept->concept_index,
        {BytesType, BytesType});

    auto *BytesOrderedType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.OrderedConcept->concept_index,
        {BytesType});

    auto *BytesInstance = state.addInstance(instancePath, lyo1::InstanceFlags::NONE, preludeSymbols.SingletonInstance);
    auto *BytesComparisonImpl = state.addImpl(instancePath, BytesComparisonType, preludeSymbols.ComparisonConcept);
    auto *BytesEqualityImpl = state.addImpl(instancePath, BytesEqualityType, preludeSymbols.EqualityConcept);
    auto *BytesOrderedImpl = state.addImpl(instancePath, BytesOrderedType, preludeSymbols.OrderedConcept);

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesCompare");
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
        state.addImplExtension("Equals", BytesEqualityImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesCompare");
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
        state.addImplExtension("LessThan", BytesComparisonImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesCompare");
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
        state.addImplExtension("GreaterThan", BytesComparisonImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        state.writeTrap(code, "BytesCompare");
        TU_RAISE_IF_NOT_OK(code.jumpIfLessOrEqual(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("LessEquals", BytesComparisonImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesCompare");
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
        state.addImplExtension("GreaterEquals", BytesComparisonImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "BytesCompare");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Compare", BytesOrderedImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, I64Type, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(BytesInstance, {}, code);
        state.setInstanceAllocator(BytesInstance, "SingletonAlloc");
    }

    return BytesInstance;
}
