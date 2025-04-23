
#include "compile_object.h"

CoreClass *
declare_core_Object(BuilderState &state, const CoreExistential *AnyExistential)
{
    uint32_t type_index = state.types.size();
    uint32_t class_index = state.classes.size();

    auto *ObjectType = new CoreType();
    ObjectType->type_index = type_index;
    ObjectType->typeAssignable = lyo1::Assignable::ConcreteAssignable;
    ObjectType->concreteSection = lyo1::TypeSection::Class;
    ObjectType->concreteDescriptor = class_index;
    ObjectType->superType = AnyExistential->existentialType;
    state.types.push_back(ObjectType);

    auto *ObjectClass = new CoreClass();
    ObjectClass->class_index = class_index;
    ObjectClass->classPath = lyric_common::SymbolPath({"Object"});
    ObjectClass->classType = ObjectType;
    ObjectClass->superClass = nullptr;
    ObjectClass->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    ObjectClass->classCtor = nullptr;
    ObjectClass->flags = lyo1::ClassFlags::NONE;
    state.classes.push_back(ObjectClass);
    state.classcache[ObjectClass->classPath] = ObjectClass;

    tu_uint32 symbol_index = state.symbols.size();
    auto *ObjectSymbol = new CoreSymbol();
    ObjectSymbol->section = lyo1::DescriptorSection::Class;
    ObjectSymbol->index = class_index;
    state.symbols.push_back(ObjectSymbol);
    TU_ASSERT (!state.symboltable.contains(ObjectClass->classPath));
    state.symboltable[ObjectClass->classPath] = symbol_index;

    return ObjectClass;
}

void
build_core_Object(BuilderState &state, const CoreClass *ObjectClass)
{
    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(ObjectClass, {}, code);
        state.setClassAllocator(ObjectClass, "ObjectAlloc");
    }
}
