
#include "compile_utf8.h"

CoreExistential *build_core_Utf8(BuilderState &state, const CoreExistential *DataExistential)
{
    lyric_common::SymbolPath existentialPath({"Utf8"});
    auto *Utf8Existential = state.addExistential(existentialPath, lyo1::IntrinsicType::Utf8,
        lyo1::ExistentialFlags::Final, DataExistential);
    return Utf8Existential;
}

//CoreInstance *build_core_Utf8Instance(BuilderState &state,
//                                       const CoreType *Utf8Type,
//                                       const CoreType *InstanceType,
//                                       const CoreInstance *SingletonInstance,
//                                       const CoreConcept *ComparisonConcept,
//                                       const CoreConcept *EqualityConcept,
//                                       const CoreConcept *OrderedConcept,
//                                       const CoreType *CharType,
//                                       const CoreType *IntegerType,
//                                       const CoreType *BoolType)
//{
//    lyric_common::SymbolPath instancePath({"Utf8Instance"});
//
//    auto *Utf8ComparisonType = state.addConcreteType(nullptr,
//        lyo1::TypeSection::Concept,
//        ComparisonConcept->concept_index,
//        {Utf8Type, Utf8Type});
//
//    auto *Utf8EqualityType = state.addConcreteType(nullptr,
//        lyo1::TypeSection::Concept,
//        EqualityConcept->concept_index,
//        {Utf8Type, Utf8Type});
//
//    auto *Utf8OrderedType = state.addConcreteType(nullptr,
//        lyo1::TypeSection::Concept,
//        OrderedConcept->concept_index,
//        {Utf8Type});
//
//    auto *Utf8Instance = state.addInstance(instancePath,
//        lyo1::InstanceFlags::NONE, SingletonInstance);
//    auto *Utf8ComparisonImpl = state.addInstanceImpl(instancePath, Utf8ComparisonType);
//    auto *Utf8EqualityImpl = state.addInstanceImpl(instancePath, Utf8EqualityType);
//    auto *Utf8OrderedImpl = state.addInstanceImpl(instancePath, Utf8OrderedType);
//
//    {
//        lyric_compiler::CodeBuilder code;
//        code.writeOpcode(lyric_runtime::Opcode::OP_TRAP);
//        code.writeU32(static_cast<uint32_t>(CoreTrap::UTF8_COMPARE));
//        auto matchDst = code.jumpIfZero();
//        code.loadBool(false);
//        auto joinDst = code.jump();
//        auto matchSrc = code.makeLabel();
//        code.patch(matchDst.getResult(), matchSrc.getResult());
//        code.loadBool(true);
//        auto nomatchSrc = code.makeLabel();
//        code.patch(joinDst.getResult(), nomatchSrc.getResult());
//        code.writeOpcode(lyric_runtime::Opcode::OP_NOOP);
//        state.addImplExtension("equals", Utf8Instance, Utf8EqualityImpl,
//            {{"lhs", Utf8Type}, {"rhs", Utf8Type}}, {},
//            code, BoolType, false);
//    }
//    {
//        lyric_compiler::CodeBuilder code;
//        code.writeOpcode(lyric_runtime::Opcode::OP_TRAP);
//        code.writeU32(static_cast<uint32_t>(CoreTrap::UTF8_COMPARE));
//        auto matchDst = code.jumpIfLessThan();
//        code.loadBool(false);
//        auto joinDst = code.jump();
//        auto matchSrc = code.makeLabel();
//        code.patch(matchDst.getResult(), matchSrc.getResult());
//        code.loadBool(true);
//        auto nomatchSrc = code.makeLabel();
//        code.patch(joinDst.getResult(), nomatchSrc.getResult());
//        code.writeOpcode(lyric_runtime::Opcode::OP_NOOP);
//        state.addImplExtension("lessthan", Utf8Instance, Utf8ComparisonImpl,
//            {{"lhs", Utf8Type}, {"rhs", Utf8Type}}, {},
//            code, BoolType, false);
//    }
//    {
//        lyric_compiler::CodeBuilder code;
//        code.writeOpcode(lyric_runtime::Opcode::OP_TRAP);
//        code.writeU32(static_cast<uint32_t>(CoreTrap::UTF8_COMPARE));
//        auto matchDst = code.jumpIfGreaterThan();
//        code.loadBool(false);
//        auto joinDst = code.jump();
//        auto matchSrc = code.makeLabel();
//        code.patch(matchDst.getResult(), matchSrc.getResult());
//        code.loadBool(true);
//        auto nomatchSrc = code.makeLabel();
//        code.patch(joinDst.getResult(), nomatchSrc.getResult());
//        code.writeOpcode(lyric_runtime::Opcode::OP_NOOP);
//        state.addImplExtension("greaterthan", Utf8Instance, Utf8ComparisonImpl,
//            {{"lhs", Utf8Type}, {"rhs", Utf8Type}}, {},
//            code, BoolType, false);
//    }
//    {
//        lyric_compiler::CodeBuilder code;
//        code.writeOpcode(lyric_runtime::Opcode::OP_TRAP);
//        code.writeU32(static_cast<uint32_t>(CoreTrap::UTF8_COMPARE));
//        auto matchDst = code.jumpIfLessOrEqual();
//        code.loadBool(false);
//        auto joinDst = code.jump();
//        auto matchSrc = code.makeLabel();
//        code.patch(matchDst.getResult(), matchSrc.getResult());
//        code.loadBool(true);
//        auto nomatchSrc = code.makeLabel();
//        code.patch(joinDst.getResult(), nomatchSrc.getResult());
//        code.writeOpcode(lyric_runtime::Opcode::OP_NOOP);
//        state.addImplExtension("lessequals", Utf8Instance, Utf8ComparisonImpl,
//            {{"lhs", Utf8Type}, {"rhs", Utf8Type}}, {},
//            code, BoolType, false);
//    }
//    {
//        lyric_compiler::CodeBuilder code;
//        code.writeOpcode(lyric_runtime::Opcode::OP_TRAP);
//        code.writeU32(static_cast<uint32_t>(CoreTrap::UTF8_COMPARE));
//        auto matchDst = code.jumpIfGreaterOrEqual();
//        code.loadBool(false);
//        auto joinDst = code.jump();
//        auto matchSrc = code.makeLabel();
//        code.patch(matchDst.getResult(), matchSrc.getResult());
//        code.loadBool(true);
//        auto nomatchSrc = code.makeLabel();
//        code.patch(joinDst.getResult(), nomatchSrc.getResult());
//        code.writeOpcode(lyric_runtime::Opcode::OP_NOOP);
//        state.addImplExtension("greaterequals", Utf8Instance, Utf8ComparisonImpl,
//            {{"lhs", Utf8Type}, {"rhs", Utf8Type}}, {},
//            code, BoolType, false);
//    }
//    {
//        lyric_compiler::CodeBuilder code;
//        code.writeOpcode(lyric_runtime::Opcode::OP_TRAP);
//        code.writeU32(static_cast<uint32_t>(CoreTrap::UTF8_COMPARE));
//        state.addImplExtension("compare", Utf8Instance, Utf8OrderedImpl,
//            {{"lhs", Utf8Type}, {"rhs", Utf8Type}}, {},
//            code, IntegerType, false);
//    }
////    {
////        CodeBuilder code;
////        code.writeOpcode(lyric_runtime::Opcode::OP_TRAP);
////        code.writeU32(static_cast<uint32_t>(CoreTrap::UTF8_AT));
////        state.addImplExtension("at", Utf8Instance, Utf8ExtendsImpl,
////            {{"utf8", Utf8Type}, {"index", IntegerType}},
////            code, Char->classType, false);
////    }
////    {
////        CodeBuilder code;
////        code.writeOpcode(lyric_runtime::Opcode::OP_TRAP);
////        code.writeU32(static_cast<uint32_t>(CoreTrap::UTF8_SIZE));
////        state.addImplExtension("size", Utf8Instance, Utf8Instance, Utf8ExtendsImpl,
////            {{"utf8", Utf8Type}},
////            code, IntegerType, false);
////    }
//    {
//        lyric_common::SymbolPath ctorPath(instancePath.getPath(), "$ctor");
//        auto *CtorTemplate = state.addTemplate(
//            ctorPath,
//            {{"T", lyo1::PlaceholderVariance::Invariant}});
//
//        auto *TType = CtorTemplate->types["T"];
//        auto *CtorTType = state.addConcreteType(InstanceType,
//            lyo1::TypeSection::Type,
//            InstanceType->type_index, {TType});
//
//        lyric_compiler::CodeBuilder code;
//        code.writeOpcode(lyric_runtime::Opcode::OP_TRAP);
//        code.writeU32(static_cast<uint32_t>(CoreTrap::SINGLETON_CTOR));
//        state.addGenericFunction(ctorPath, CtorTemplate,
//            {{"$descriptor", CtorTType}}, {},
//            code, Utf8Instance->instanceType);
//    }
//
//    return Utf8Instance;
//}
