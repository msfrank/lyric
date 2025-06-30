
#include "compile_rest.h"

CoreExistential *
declare_core_Rest(BuilderState &state, const CoreExistential *AnyExistential)
{
    lyric_common::SymbolPath existentialPath({"Rest"});

    std::vector<CorePlaceholder> placeholders;
    placeholders.push_back({"T", lyo1::PlaceholderVariance::Invariant});
    auto *RestTemplate = state.addTemplate(existentialPath, placeholders);

    auto *RestExistential = state.addGenericExistential(existentialPath, RestTemplate,
        lyo1::IntrinsicType::Invalid, lyo1::ExistentialFlags::Final, AnyExistential);
    return RestExistential;
}

void
build_core_Rest(
    BuilderState &state,
    const CoreExistential *RestExistential,
    const CoreConcept *IterableConcept,
    const CoreConcept *IteratorConcept,
    const CoreClass *RestIteratorClass,
    const CoreType *IntType,
    const CoreType *NilType)
{
    auto *TType = RestExistential->existentialTemplate->types.at("T");
    auto *TOrNilType = state.addUnionType({TType,NilType});

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "RestSize");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Size",
            RestExistential,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code, IntType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "RestGet");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Get",
            RestExistential,
            lyo1::CallFlags::GlobalVisibility,
            {
                make_list_param("index", IntType),
            },
            code, TOrNilType);
    }

    auto *IterableTType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        IterableConcept->concept_index, {TType});

    auto *IterableImpl = state.addImpl(RestExistential->existentialPath, IterableTType, IterableConcept);

    {
        auto *IteratorTType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
            IteratorConcept->concept_index, {TType});
        lyric_object::BytecodeBuilder code;
        code.loadClass(RestIteratorClass->class_index);
        state.writeTrap(code, "RestIterate");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Iterate", IterableImpl, {}, code, IteratorTType);
    }
}

CoreClass *
build_core_RestIterator(
    BuilderState &state,
    const CoreClass *ObjectClass,
    const CoreConcept *IteratorConcept,
    const CoreType *BoolType)
{
    lyric_common::SymbolPath classPath({"RestIterator"});

    auto *RestIteratorTemplate = state.addTemplate(
        classPath,
        {
            {"T", lyo1::PlaceholderVariance::Contravariant},
        });

    auto *TType = RestIteratorTemplate->types["T"];

    auto *RestIteratorClass = state.addGenericClass(classPath, RestIteratorTemplate,
        lyo1::ClassFlags::Final, ObjectClass);

    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(RestIteratorClass,
            {},
            code);
        state.setClassAllocator(RestIteratorClass, "RestIteratorAlloc");
    }

    auto *IteratorTType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        IteratorConcept->concept_index, {TType});

    auto *IteratorImpl = state.addImpl(classPath, IteratorTType, IteratorConcept);

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "RestIteratorValid");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Valid", IteratorImpl, {}, code, BoolType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "RestIteratorNext");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Next", IteratorImpl, {}, code, TType);
    }

    return RestIteratorClass;
}