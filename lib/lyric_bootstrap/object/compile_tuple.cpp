
#include "compile_tuple.h"

CoreClass *
declare_core_TupleN(BuilderState &state, int arity, const PreludeSymbols &preludeSymbols)
{
    TU_ASSERT (arity > 0);

    lyric_common::SymbolPath classPath({absl::StrCat("Tuple", arity)});

    std::vector<CorePlaceholder> placeholders;
    for (int i = 0; i < arity; i++) {
        auto name = absl::StrCat("T", i);
        CorePlaceholder p;
        placeholders.push_back({name, lyo1::PlaceholderVariance::Contravariant});
    }

    auto *TupleNTemplate = state.addTemplate(classPath, placeholders);
    auto *TupleNClass = state.addGenericClass(classPath, TupleNTemplate, lyo1::ClassFlags::NONE, preludeSymbols.ObjectClass);
    return TupleNClass;
}

void
build_core_TupleN(BuilderState &state, int arity, const PreludeSymbols &preludeSymbols)
{
    TU_ASSERT (arity > 0);

    auto *TupleNClass = preludeSymbols.tupleClasses[arity - 1];
    auto *TupleNTemplate = TupleNClass->classTemplate;
    auto *UnwrapNConcept = preludeSymbols.unwrapConcepts[arity - 1];

    //
    std::vector<tu_uint32> tupleFields(arity);
    for (int i = 0; i < arity; i++) {
        auto name = absl::StrCat("Element", i);
        auto *TType = TupleNTemplate->types.at(TupleNTemplate->names.at(i));
        auto *TupleField = state.addClassMember(name, TupleNClass,
            lyo1::FieldFlags::NONE, TType);
        tupleFields[i] = TupleField->field_index;
    }

    // TupleN ctor
    {
        std::vector<CoreParam> ctorParams;
        std::vector<const CoreType *> ctorTypeParams;
        for (int i = 0; i < arity; i++) {
            auto name = absl::StrCat("t", i);
            auto *TType = TupleNTemplate->types.at(TupleNTemplate->names.at(i));
            ctorParams.push_back(make_list_param(name, TType));
            ctorTypeParams.push_back(TType);
        }

        lyric_object::BytecodeBuilder code;
        code.loadReceiver();
        // call the parent ctor
        code.callVirtual(TupleNClass->superClass->classCtor->call_index, 0);
        // load the remaining TupleN.$ctor arguments and store them as members
        for (tu_uint32 i = 0; i < ctorParams.size(); i++) {
            code.loadReceiver();
            code.loadArgument(i);
            code.storeField(tupleFields[i]);
        }
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(TupleNClass, ctorParams, code);
    }

    std::vector<const CoreType *> unwrapTypes;
    unwrapTypes.push_back(TupleNClass->classType);
    for (int i = 0; i < TupleNTemplate->placeholders.size(); i++) {
        const auto *elementType = TupleNTemplate->types.at(absl::StrCat("T", i));
        unwrapTypes.push_back(elementType);
    }

    auto *TupleNUnwrapType = state.addConcreteType(
        nullptr, lyo1::TypeSection::Concept, UnwrapNConcept->concept_index, unwrapTypes);

    auto *TupleNUnwrapImpl = state.addImpl(TupleNClass->classPath, TupleNUnwrapType, UnwrapNConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK (code.loadArgument(0));
        TU_RAISE_IF_NOT_OK (code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Unwrap", TupleNUnwrapImpl,
            {
                make_list_param("wrapped", TupleNClass->classType),
            },
            code, TupleNClass->classType, false);
    }
}

const CoreInstance *
build_core_TupleNInstance(BuilderState &state, int arity, const PreludeSymbols &preludeSymbols)
{
    auto *TupleNClass = preludeSymbols.tupleClasses[arity - 1];
    auto *UnwrapNConcept = preludeSymbols.unwrapConcepts[arity - 1];

    lyric_common::SymbolPath instancePath({absl::StrCat(TupleNClass->classPath.getName(), "Instance")});

    auto *TupleUnwrapType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        UnwrapNConcept->concept_index,
        {TupleNClass->classType, TupleNClass->classType});

    auto *TupleInstance = state.addInstance(instancePath,
        lyo1::InstanceFlags::NONE, preludeSymbols.SingletonInstance);
    auto *TupleUnwrapImpl = state.addImpl(instancePath, TupleUnwrapType, UnwrapNConcept);

    {
        lyric_object::BytecodeBuilder code;
        code.loadArgument(0);
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addImplExtension("Unwrap", TupleUnwrapImpl,
            {
                make_list_param("wrapped", TupleNClass->classType),
            },
            code, TupleNClass->classType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addInstanceCtor(TupleInstance, {}, code);
        state.setInstanceAllocator(TupleInstance, "SingletonAlloc");
    }

    return TupleInstance;
}
