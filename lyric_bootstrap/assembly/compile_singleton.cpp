
#include "compile_singleton.h"

CoreInstance *
declare_core_Singleton(BuilderState &state, const CoreExistential *AnyExistential)
{
    uint32_t type_index = state.types.size();
    uint32_t instance_index = state.instances.size();

    auto *SingletonType = new CoreType();
    SingletonType->type_index = type_index;
    SingletonType->typeAssignable = lyo1::Assignable::ConcreteAssignable;
    SingletonType->concreteSection = lyo1::TypeSection::Instance;
    SingletonType->concreteDescriptor = instance_index;
    SingletonType->superType = AnyExistential->existentialType;
    state.types.push_back(SingletonType);

    auto *SingletonInstance = new CoreInstance();
    SingletonInstance->instance_index = instance_index;
    SingletonInstance->instancePath = lyric_common::SymbolPath({"Singleton"});
    SingletonInstance->instanceType = SingletonType;
    SingletonInstance->superInstance = nullptr;
    SingletonInstance->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    SingletonInstance->instanceCtor = nullptr;
    SingletonInstance->flags = lyo1::InstanceFlags::NONE;
    state.instances.push_back(SingletonInstance);
    state.instancecache[SingletonInstance->instancePath] = SingletonInstance;

    auto *SingletonSymbol = new CoreSymbol();
    SingletonSymbol->symbolPath = SingletonInstance->instancePath;
    SingletonSymbol->section = lyo1::DescriptorSection::Instance;
    SingletonSymbol->index = instance_index;
    TU_ASSERT (!state.symbols.contains(SingletonSymbol->symbolPath));
    state.symbols[SingletonSymbol->symbolPath] = SingletonSymbol;

    return SingletonInstance;
}

void
build_core_Singleton(BuilderState &state, const CoreInstance *SingletonInstance)
{
    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addInstanceCtor(SingletonInstance, {}, code);
        state.setInstanceAllocator(SingletonInstance, lyric_bootstrap::internal::BootstrapTrap::SINGLETON_ALLOC);
    }
}
