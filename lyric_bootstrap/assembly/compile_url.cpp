
#include "compile_url.h"

const CoreStruct *
build_core_Url(
    BuilderState &state,
    const CoreStruct *RecordStruct,
    const CoreType *Utf8Type,
    const CoreType *IntegerType,
    const CoreType *CharType)
{
    lyric_common::SymbolPath structPath({"Url"});

    auto *UrlStruct = state.addStruct(structPath, lyo1::StructFlags::Final, RecordStruct);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::URI_CTOR)));
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addStructCtor(UrlStruct,
            {
                {"utf8", Utf8Type, nullptr, lyo1::ParameterFlags::NONE},
            },
            code);
        state.setStructAllocator(UrlStruct, lyric_bootstrap::internal::BootstrapTrap::URI_ALLOC);
    }

    return UrlStruct;
}

CoreInstance *
build_core_UrlInstance(
    BuilderState &state,
    const CoreType *UrlType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *EqualityConcept,
    const CoreType *IntegerType,
    const CoreType *BoolType)
{
    lyric_common::SymbolPath instancePath({"UrlInstance"});

    auto *UrlEqualityType = state.addConcreteType(nullptr,
        lyo1::TypeSection::Concept,
        EqualityConcept->concept_index,
        {UrlType, UrlType});

    auto *UrlInstance = state.addInstance(instancePath, lyo1::InstanceFlags::NONE, SingletonInstance);
    auto *UrlEqualityImpl = state.addImpl(instancePath, UrlEqualityType, EqualityConcept);

    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::URI_EQUALS)));
        state.addImplExtension("equals", UrlEqualityImpl,
            {{"lhs", UrlType}, {"rhs", UrlType}}, {},
            code, BoolType, false);
    }
    {
        lyric_object::BytecodeBuilder code;
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addInstanceCtor(UrlInstance, {}, code);
        state.setInstanceAllocator(UrlInstance, lyric_bootstrap::internal::BootstrapTrap::SINGLETON_ALLOC);
    }

    return UrlInstance;
}
