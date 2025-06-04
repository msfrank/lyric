
#include "compile_string.h"

CoreExistential *
declare_core_String(BuilderState &state, const CoreExistential *IntrinsicExistential)
{
    lyric_common::SymbolPath existentialPath({"String"});
    auto *StringExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::String,
        lyo1::ExistentialFlags::Final, IntrinsicExistential);
    return StringExistential;
}

void
build_core_String(
    BuilderState &state,
    const CoreExistential *StringExistential,
    const CoreType *IntType,
    const CoreType *CharType,
    const CoreType *BytesType)
{
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringLength");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Length",
            StringExistential,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code, IntType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringAt");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("At",
            StringExistential,
            lyo1::CallFlags::GlobalVisibility,
            {
                make_list_param("index", IntType),
            },
            code, CharType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StringToBytes");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("ToBytes",
            StringExistential,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code, BytesType);
    }
}

CoreInstance *
build_core_StringInstance(
    BuilderState &state,
    const CoreType *StringType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *CharType,
    const CoreType *IntegerType,
    const CoreType *BoolType)
{
    lyric_common::SymbolPath instancePath({"StringInstance"});

    auto *StringComparisonType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        ComparisonConcept->concept_index,
        {StringType, StringType});

    auto *StringEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        EqualityConcept->concept_index,
        {StringType, StringType});

    auto *StringOrderedType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        OrderedConcept->concept_index,
        {StringType});

    auto *StringInstance = state.addInstance(instancePath, lyo1::InstanceFlags::NONE, SingletonInstance);
    auto *StringComparisonImpl = state.addImpl(instancePath, StringComparisonType, ComparisonConcept);
    auto *StringEqualityImpl = state.addImpl(instancePath, StringEqualityType, EqualityConcept);
    auto *StringOrderedImpl = state.addImpl(instancePath, StringOrderedType, OrderedConcept);

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
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
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
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
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
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
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
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
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
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
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
        state.addImplExtension("Compare", StringOrderedImpl,
            {
                make_list_param("lhs", StringType),
                make_list_param("rhs", StringType),
            },
            code, IntegerType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(StringInstance, {}, code);
        state.setInstanceAllocator(StringInstance, "SingletonAlloc");
    }

    return StringInstance;
}
