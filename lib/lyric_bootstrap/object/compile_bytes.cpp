
#include "compile_bytes.h"

CoreExistential *
declare_core_Bytes(BuilderState &state, const CoreExistential *IntrinsicExistential)
{
    lyric_common::SymbolPath existentialPath({"Bytes"});
    auto *BytesExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Bytes,
        lyo1::ExistentialFlags::Final, IntrinsicExistential);
    return BytesExistential;
}

void
build_core_Bytes(
    BuilderState &state,
    const CoreExistential *BytesExistential,
    const CoreType *IntType)
{
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::BYTES_LENGTH)));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Length",
            BytesExistential,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code, IntType);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::BYTES_AT)));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("At",
            BytesExistential,
            lyo1::CallFlags::GlobalVisibility,
            {
                make_list_param("index", IntType),
            },
            code, IntType);
    }
}

CoreInstance *
build_core_BytesInstance(
    BuilderState &state,
    const CoreType *BytesType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *IntegerType,
    const CoreType *BoolType)
{
    lyric_common::SymbolPath instancePath({"BytesInstance"});

    auto *BytesComparisonType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        ComparisonConcept->concept_index,
        {BytesType, BytesType});

    auto *BytesEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        EqualityConcept->concept_index,
        {BytesType, BytesType});

    auto *BytesOrderedType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        OrderedConcept->concept_index,
        {BytesType});

    auto *BytesInstance = state.addInstance(instancePath, lyo1::InstanceFlags::NONE, SingletonInstance);
    auto *BytesComparisonImpl = state.addImpl(instancePath, BytesComparisonType, ComparisonConcept);
    auto *BytesEqualityImpl = state.addImpl(instancePath, BytesEqualityType, EqualityConcept);
    auto *BytesOrderedImpl = state.addImpl(instancePath, BytesOrderedType, OrderedConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::BYTES_COMPARE)));
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
        state.addImplExtension("equals", BytesEqualityImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::BYTES_COMPARE)));
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
        state.addImplExtension("lessthan", BytesComparisonImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::BYTES_COMPARE)));
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
        state.addImplExtension("greaterthan", BytesComparisonImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        tu_uint16 matchDst, joinDst, matchSrc, nomatchSrc;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::BYTES_COMPARE)));
        TU_RAISE_IF_NOT_OK(code.jumpIfLessOrEqual(matchDst));
        TU_RAISE_IF_NOT_OK(code.loadBool(false));
        TU_RAISE_IF_NOT_OK(code.jump(joinDst));
        TU_RAISE_IF_NOT_OK(code.makeLabel(matchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(matchDst, matchSrc));
        TU_RAISE_IF_NOT_OK(code.loadBool(true));
        TU_RAISE_IF_NOT_OK(code.makeLabel(nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.patch(joinDst, nomatchSrc));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_NOOP));
        state.addImplExtension("lessequals", BytesComparisonImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::BYTES_COMPARE)));
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
        state.addImplExtension("greaterequals", BytesComparisonImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::BYTES_COMPARE)));
        state.addImplExtension("compare", BytesOrderedImpl,
            {
                make_list_param("lhs", BytesType),
                make_list_param("rhs", BytesType),
            },
            code, IntegerType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(BytesInstance, {}, code);
        state.setInstanceAllocator(BytesInstance, lyric_bootstrap::internal::BootstrapTrap::SINGLETON_ALLOC);
    }

    return BytesInstance;
}