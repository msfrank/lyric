
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
    const CoreType *CharType)
{
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::STRING_LENGTH)));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Length",
            StringExistential,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code, IntType);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::STRING_AT)));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("At",
            StringExistential,
            lyo1::CallFlags::GlobalVisibility,
            {
                {"index", IntType, nullptr, lyo1::ParameterFlags::NONE}
            },
            code, CharType);
    }
}

//const CoreStruct *
//build_core_String(
//    BuilderState &state,
//    const CoreStruct *RecordStruct,
//    const CoreType *Utf8Type,
//    const CoreType *IntegerType,
//    const CoreType *CharType)
//{
//    lyric_common::SymbolPath structPath({"String"});
//
//    auto *StringStruct = state.addStruct(structPath, lyo1::StructFlags::Final, RecordStruct);
//
//    {
//        lyric_object::BytecodeBuilder code;
//        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::STRING_CTOR)));
//        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
//        state.addStructCtor(StringStruct,
//            {
//                {"utf8", Utf8Type, nullptr, lyo1::ParameterFlags::NONE},
//            },
//            code);
//        state.setStructAllocator(StringStruct, lyric_bootstrap::internal::BootstrapTrap::STRING_ALLOC);
//    }

//
//    return StringStruct;
//}

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
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::STRING_COMPARE)));
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
        state.addImplExtension("equals", StringEqualityImpl,
            {{"lhs", StringType}, {"rhs", StringType}}, {},
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::STRING_COMPARE)));
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
        state.addImplExtension("lessthan", StringComparisonImpl,
            {{"lhs", StringType}, {"rhs", StringType}}, {},
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::STRING_COMPARE)));
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
        state.addImplExtension("greaterthan", StringComparisonImpl,
            {{"lhs", StringType}, {"rhs", StringType}}, {},
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::STRING_COMPARE)));
        TU_RAISE_IF_NOT_OK(code.jumpIfLessOrEqual(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
        state.addImplExtension("lessequals", StringComparisonImpl,
            {{"lhs", StringType}, {"rhs", StringType}}, {},
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::STRING_COMPARE)));
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
        state.addImplExtension("greaterequals", StringComparisonImpl,
            {{"lhs", StringType}, {"rhs", StringType}}, {},
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::STRING_COMPARE)));
        state.addImplExtension("compare", StringOrderedImpl,
            {{"lhs", StringType}, {"rhs", StringType}}, {},
            code, IntegerType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(StringInstance, {}, code);
        state.setInstanceAllocator(StringInstance, lyric_bootstrap::internal::BootstrapTrap::SINGLETON_ALLOC);
    }

    return StringInstance;
}
