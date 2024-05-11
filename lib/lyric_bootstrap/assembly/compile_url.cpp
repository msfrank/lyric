
#include "compile_url.h"

CoreExistential *
declare_core_Url(BuilderState &state, const CoreExistential *IntrinsicExistential)
{
    lyric_common::SymbolPath existentialPath({"Url"});
    auto *UrlExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Url,
        lyo1::ExistentialFlags::Final, IntrinsicExistential);
    return UrlExistential;
}

void
build_core_Url(BuilderState &state, const CoreExistential *UrlExistential)
{
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
        TU_RAISE_IF_NOT_OK(code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::URL_EQUALS)));
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
