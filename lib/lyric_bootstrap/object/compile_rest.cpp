
#include "compile_rest.h"
#include "prelude_symbols.h"

CoreExistential *
declare_core_Rest(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Rest"});

    std::vector<CorePlaceholder> placeholders;
    placeholders.push_back({"T", lyo1::PlaceholderVariance::Invariant});
    auto *RestTemplate = state.addTemplate(existentialPath, placeholders);

    auto *RestExistential = state.addGenericExistential(
        existentialPath, RestTemplate, lyo1::ExistentialFlags::Final, preludeSymbols.AnyExistential);
    return RestExistential;
}

void
build_core_Rest(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *UndefType = preludeSymbols.UndefExistential->existentialType;

    auto *RestExistential = preludeSymbols.RestExistential;
    auto *TType = RestExistential->existentialTemplate->types.at("T");
    auto *TOrUndefType = state.addUnionType({TType,UndefType});

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "RestSize");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Size",
            RestExistential,
            lyo1::CallFlags::NONE,
            {},
            code, I64Type);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "RestGet");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("Get",
            RestExistential,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", I64Type),
            },
            code, TOrUndefType);
    }

    auto *RestIterableType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        preludeSymbols.IterableConcept->concept_index, {RestExistential->existentialType, TType});

    auto *IterableImpl = state.addImpl(RestExistential->existentialPath, RestIterableType, preludeSymbols.IterableConcept);

    {
        auto *IteratorTType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
            preludeSymbols.IteratorConcept->concept_index, {TType});
        lyric_object::BytecodeBuilder code;
        code.loadClass(preludeSymbols.RestIteratorClass->class_index);
        state.writeTrap(code, "RestIterate");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Iterate", IterableImpl,
            {
                make_list_param("source", RestExistential->existentialType),
            },
            code,
            IteratorTType);
    }
}

CoreClass *
build_core_RestIterator(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *BoolType = preludeSymbols.BoolExistential->existentialType;

    lyric_common::SymbolPath classPath({"RestIterator"});

    auto *RestIteratorTemplate = state.addTemplate(
        classPath,
        {
            {"T", lyo1::PlaceholderVariance::Contravariant},
        });

    auto *TType = RestIteratorTemplate->types["T"];

    auto *RestIteratorClass = state.addGenericClass(classPath, RestIteratorTemplate,
        lyo1::ClassFlags::Final, preludeSymbols.ObjectClass);

    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(RestIteratorClass,
            {},
            code);
        state.setClassAllocator(RestIteratorClass, "RestIteratorAlloc");
    }

    auto *IteratorTType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        preludeSymbols.IteratorConcept->concept_index, {TType});

    auto *IteratorImpl = state.addImpl(classPath, IteratorTType, preludeSymbols.IteratorConcept);

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