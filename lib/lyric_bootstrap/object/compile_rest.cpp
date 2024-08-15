//
//#include "core_rest.h"
//
//CoreClass *build_core_Rest(BuilderState &state,
//                           const CoreClass *ObjectClass,
//                           const CoreConcept *Varargs,
//                           const CoreClass *Integer)
//{
//    SymbolPath classPath("Rest");
//
//    auto *RestTemplate = state.addTemplate(
//        classPath,
//        {
//            {"T", lya1::PlaceholderVariance::Invariant}
//        });
//
////    auto *TType = RestTemplate->types["T"];
//
//    auto *RestClass = state.addGenericClass(classPath, RestTemplate,
//        lya1::ClassFlags::Singleton | lya1::ClassFlags::Final, ObjectClass);
//
////    {
////        CodeBuilder code;
////        code.writeOpcode(Opcode::OP_VA_LOAD);
////        state.addStaticMethod("at", RestClass,
////                              {{"i", Integer->classType}},
////                              code, TType, true);
////    }
////    {
////        CodeBuilder code;
////        code.writeOpcode(Opcode::OP_VA_SIZE);
////        state.addStaticMethod("size", RestClass,
////                              {},
////                              code, Integer->classType, true);
////    }
////
////    auto *RestVarargsMagnet = new CoreType();
////    RestVarargsMagnet->type_index = state.types.size();
////    RestVarargsMagnet->superType = nullptr;
////    RestVarargsMagnet->typeId = lya1::TypeId(lya1::TypeSection::CONCEPT_SECTION, Varargs->concept_index);
////    RestVarargsMagnet->typeParameters = {RestClass->classType->type_index};
////    state.types.push_back(RestVarargsMagnet);
////
////    uint32_t impl_index = state.impls.size();
////    auto *RestImplType = new CoreType();
////    RestImplType->type_index = state.types.size();
////    RestImplType->typeId = lya1::TypeId(lya1::TypeSection::IMPL_SECTION, impl_index);
////    RestImplType->superType = nullptr;
////    state.types.push_back(RestImplType);
////
////    auto *RestImpl = new CoreInstance();
////    RestImpl->impl_index = impl_index;
////    RestImpl->implPath = SymbolPath("RestImpl");
////    RestImpl->superImpl = nullptr;
////    RestImpl->implType = RestImplType;
////    RestImpl->magnets.emplace_back(lya1::Magnet(RestVarargsMagnet->type_index, lya1::MagnetFlags::NONE));
////    state.impls.push_back(RestImpl);
////
////    auto *RestImplSymbol = new CoreSymbol();
////    RestImplSymbol->symbolPath = RestImpl->implPath;
////    RestImplSymbol->section = lya1::DescriptorSection::IMPL_SECTION;
////    RestImplSymbol->index = RestImpl->impl_index;
////    state.symbols[RestImplSymbol->symbolPath] = RestImplSymbol;
//
//    return RestClass;
//}
