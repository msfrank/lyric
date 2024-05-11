
#include "compile_tuple.h"

CoreClass *
build_core_TupleN(BuilderState &state, int arity, const CoreClass *ObjectClass)
{
    TU_ASSERT (arity > 0);

    lyric_common::SymbolPath classPath({absl::StrCat("Tuple", arity)});

    std::vector<CorePlaceholder> placeholders;
    for (int i = 0; i < arity; i++) {
        auto name = absl::StrCat("T", i);
        CorePlaceholder p;
        placeholders.push_back({name, lyo1::PlaceholderVariance::Contravariant});
    }

    auto *TupleTemplate = state.addTemplate(classPath, placeholders);

    auto *TupleClass = state.addGenericClass(classPath, TupleTemplate,
        lyo1::ClassFlags::NONE, ObjectClass);

    //
    absl::flat_hash_map<std::string,tu_uint32> tupleFields;
    for (int i = 0; i < arity; i++) {
        auto name = absl::StrCat("t", i);
        auto *TType = TupleTemplate->types[TupleTemplate->names[i]];
        auto *TupleField = state.addClassMember(name, TupleClass,
            lyo1::FieldFlags::GlobalVisibility, TType);
        tupleFields[name] = TupleField->field_index;
    }

    // TupleN ctor
    {
        std::vector<CoreParam> ctorParams;
        std::vector<const CoreType *> ctorTypeParams;
        for (int i = 0; i < arity; i++) {
            auto name = absl::StrCat("t", i);
            auto *TType = TupleTemplate->types[TupleTemplate->names[i]];
            ctorParams.push_back({name, TType, nullptr, lyo1::ParameterFlags::NONE});
            ctorTypeParams.push_back(TType);
        }

        lyric_object::BytecodeBuilder code;
        code.loadReceiver();
        // call the parent ctor
        code.callVirtual(TupleClass->superClass->classCtor->call_index, 0);
        // load the remaining TupleN.$ctor arguments and store them as members
        for (tu_uint32 i = 0; i < ctorParams.size(); i++) {
            code.loadArgument(i);
            code.storeField(tupleFields[ctorParams[i].paramName]);
        }
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(TupleClass, ctorParams, code);
    }

    return TupleClass;
}

const CoreInstance *
build_core_TupleNInstance(
    BuilderState &state,
    const CoreClass *TupleNClass,
    const CoreInstance *SingletonInstance,
    const CoreConcept *UnwrapConcept)
{
    lyric_common::SymbolPath instancePath({absl::StrCat(TupleNClass->classPath.getName(), "Instance")});

    auto *TupleUnwrapType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        UnwrapConcept->concept_index,
        {TupleNClass->classType, TupleNClass->classType});

    auto *TupleInstance = state.addInstance(instancePath,
        lyo1::InstanceFlags::NONE, SingletonInstance);
    auto *TupleUnwrapImpl = state.addImpl(instancePath, TupleUnwrapType, UnwrapConcept);

    {
        lyric_object::BytecodeBuilder code;
        code.loadArgument(0);
        state.addImplExtension("unwrap", TupleUnwrapImpl,
            {{"wrapped", TupleNClass->classType}}, {},
            code, TupleNClass->classType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addInstanceCtor(TupleInstance, {}, code);
        state.setInstanceAllocator(TupleInstance, lyric_bootstrap::internal::BootstrapTrap::SINGLETON_ALLOC);
    }

    return TupleInstance;
}
