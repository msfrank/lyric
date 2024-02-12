//
//#include "core_closure.h"
//#include "core_types.h"
//
//CoreClass *build_core_Closure(BuilderState &state,
//                              const CoreClass *ObjectClass,
//                              const CoreType *ClassType,
//                              const CoreType *CallType)
//{
//    SymbolPath classPath("Closure");
//
//    auto *ClosureClass = state.addClass(classPath, lya1::ClassFlags::NONE, ObjectClass);
//
//    {
//        SymbolPath ctorPath(classPath.getPath(), "$ctor");
//        auto *CtorTemplate = state.addTemplate(
//            ctorPath,
//            {{"T", lya1::PlaceholderVariance::Invariant}});
//
//        auto *TType = CtorTemplate->types["T"];
//        auto *CtorTType = state.addConcreteType(ClassType, lya1::TypeSection::Type,
//            ClassType->type_index, {TType});
//
//        CodeBuilder code;
//        code.writeOpcode(Opcode::OP_TRAP);
//        code.writeU32(static_cast<uint32_t>(CoreTrap::CLOSURE_CTOR));
//        code.writeOpcode(Opcode::OP_RETURN);
//        state.addGenericFunction(ctorPath,
//                                 CtorTemplate,
//                                 {{"$descriptor", CtorTType}, {"$call", CallType}},
//                                 code,
//                                 TType);
//    }
//
//    return ClosureClass;
//}
