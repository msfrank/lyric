
#include <lyric_common/symbol_url.h>

#include "builder_state.h"

const CoreExistential *
declare_core_Bool(BuilderState &state, const CoreExistential *IntrinsicExistential)
{
    lyric_common::SymbolPath existentialPath({"Bool"});
    auto *BoolExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Bool,
        lyo1::ExistentialFlags::Final, IntrinsicExistential);
    return BoolExistential;
}

void
build_core_Bool(BuilderState &state, const CoreExistential *BoolExistential)
{
}

const CoreInstance *
build_core_BoolInstance(
    BuilderState &state,
    const CoreType *BoolType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreConcept *PropositionConcept,
    const CoreType *IntegerType)
{
    lyric_common::SymbolPath instancePath({"BoolInstance"});

    auto *BoolEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        EqualityConcept->concept_index,
        {BoolType, BoolType});

    auto *BoolOrderedType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        OrderedConcept->concept_index,
        {BoolType});

    auto *BoolPropositionType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        PropositionConcept->concept_index,
        {BoolType, BoolType});

    auto *BoolInstance = state.addInstance(instancePath,
        lyo1::InstanceFlags::NONE, SingletonInstance);
    auto *BoolEqualityImpl = state.addImpl(instancePath, BoolEqualityType, EqualityConcept);
    auto *BoolOrderedImpl = state.addImpl(instancePath, BoolOrderedType, OrderedConcept);
    auto *BoolPropositionImpl = state.addImpl(instancePath, BoolPropositionType, PropositionConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_BOOL_CMP));
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
        state.addImplExtension("equals", BoolEqualityImpl,
            {{"lhs", BoolType}, {"rhs", BoolType}}, {},
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_BOOL_CMP));
        state.addImplExtension("compare", BoolOrderedImpl,
            {{"lhs", BoolType}, {"rhs", BoolType}}, {},
            code, IntegerType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_LOGICAL_AND));
        state.addImplExtension("conjunct", BoolPropositionImpl,
            {{"lhs", BoolType}, {"rhs", BoolType}}, {},
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_LOGICAL_OR));
        state.addImplExtension("disjunct", BoolPropositionImpl,
            {{"lhs", BoolType}, {"rhs", BoolType}}, {},
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_LOGICAL_NOT));
        state.addImplExtension("complement", BoolPropositionImpl,
            {{"lhs", BoolType}}, {},
            code, BoolType, true);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(BoolInstance, {}, code);
        state.setInstanceAllocator(BoolInstance, lyric_bootstrap::internal::BootstrapTrap::SINGLETON_ALLOC);
    }

    return BoolInstance;
}
