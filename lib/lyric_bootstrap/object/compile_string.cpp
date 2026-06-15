
#include "compile_string.h"
#include "prelude_symbols.h"

CoreExistential *
declare_core_String(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"String"});
    auto *StringExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return StringExistential;
}

void
build_core_String(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *CharType = preludeSymbols.CharExistential->existentialType;
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *StringType = preludeSymbols.StringExistential->existentialType;
    auto *BytesType = preludeSymbols.BytesExistential->existentialType;
    auto *UndefType = preludeSymbols.UndefExistential->existentialType;

    auto *CharOrUndefType = state.addUnionType({CharType,UndefType});
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringLength");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Length",
            preludeSymbols.StringExistential,
            lyo1::CallFlags::NONE,
            {},
            code, I64Type);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringAt");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("At",
            preludeSymbols.StringExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", I64Type),
            },
            code, CharOrUndefType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringToBytes");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("ToBytes",
            preludeSymbols.StringExistential,
            lyo1::CallFlags::NONE,
            {},
            code, BytesType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringAppend");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Append",
            preludeSymbols.StringExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("other", StringType),
            },
            code, StringType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringPrepend");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Prepend",
            preludeSymbols.StringExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("other", StringType),
            },
            code, StringType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringInsert");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Insert",
            preludeSymbols.StringExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", I64Type),
                make_list_param("other", StringType),
            },
            code, StringType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringRemove");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Remove",
            preludeSymbols.StringExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", I64Type),
                make_list_param("count", I64Type),
            },
            code, StringType);
    }
    // {
    //     lyric_object::BytecodeBuilder code;
    //     state.writeTrap(code, "StringSplit");
    //     TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
    //     state.addExistentialMethod("Split",
    //         StringExistential,
    //         lyo1::CallFlags::NONE,
    //         {
    //             make_list_param("index", I64Type),
    //         },
    //         code, StringType);
    // }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringSubstring");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Substring",
            preludeSymbols.StringExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", I64Type),
                make_list_param("count", I64Type),
            },
            code, StringType);
    }
}

CoreInstance *
build_core_StringInstance(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *BoolType = preludeSymbols.BoolExistential->existentialType;
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *StringType = preludeSymbols.StringExistential->existentialType;

    lyric_common::SymbolPath instancePath({"StringInstance"});

    auto *StringComparisonType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.ComparisonConcept->concept_index,
        {StringType, StringType});

    auto *StringEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.EqualityConcept->concept_index,
        {StringType, StringType});

    auto *StringOrderedType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        preludeSymbols.OrderedConcept->concept_index,
        {StringType});

    auto *StringInstance = state.addInstance(instancePath, lyo1::InstanceFlags::NONE, preludeSymbols.SingletonInstance);
    auto *StringComparisonImpl = state.addImpl(instancePath, StringComparisonType, preludeSymbols.ComparisonConcept);
    auto *StringEqualityImpl = state.addImpl(instancePath, StringEqualityType, preludeSymbols.EqualityConcept);
    auto *StringOrderedImpl = state.addImpl(instancePath, StringOrderedType, preludeSymbols.OrderedConcept);

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringCompare");
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
        state.addImplExtension("Equals", StringEqualityImpl,
            {
                make_list_param("lhs", StringType),
                make_list_param("rhs", StringType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringCompare");
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
        state.addImplExtension("LessThan", StringComparisonImpl,
            {
                make_list_param("lhs", StringType),
                make_list_param("rhs", StringType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringCompare");
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
        state.addImplExtension("GreaterThan", StringComparisonImpl,
            {
                make_list_param("lhs", StringType),
                make_list_param("rhs", StringType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        state.writeTrap(code, "StringCompare");
        TU_RAISE_IF_NOT_OK(code.jumpIfLessOrEqual(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("LessEquals", StringComparisonImpl,
            {
                make_list_param("lhs", StringType),
                make_list_param("rhs", StringType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringCompare");
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
        state.addImplExtension("GreaterEquals", StringComparisonImpl,
            {
                make_list_param("lhs", StringType),
                make_list_param("rhs", StringType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringCompare");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Compare", StringOrderedImpl,
            {
                make_list_param("lhs", StringType),
                make_list_param("rhs", StringType),
            },
            code, I64Type, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(StringInstance, {}, code);
        state.setInstanceAllocator(StringInstance, "SingletonAlloc");
    }

    return StringInstance;
}
