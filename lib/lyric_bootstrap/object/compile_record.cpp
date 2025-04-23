
#include "compile_singleton.h"

CoreStruct *
declare_core_Record(BuilderState &state, const CoreExistential *AnyExistential)
{
    uint32_t type_index = state.types.size();
    uint32_t struct_index = state.structs.size();

    auto *RecordType = new CoreType();
    RecordType->type_index = type_index;
    RecordType->typeAssignable = lyo1::Assignable::ConcreteAssignable;
    RecordType->concreteSection = lyo1::TypeSection::Struct;
    RecordType->concreteDescriptor = struct_index;
    RecordType->superType = AnyExistential->existentialType;
    state.types.push_back(RecordType);

    auto *RecordStruct = new CoreStruct();
    RecordStruct->struct_index = struct_index;
    RecordStruct->structPath = lyric_common::SymbolPath({"Record"});
    RecordStruct->structType = RecordType;
    RecordStruct->superStruct = nullptr;
    RecordStruct->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    RecordStruct->structCtor = nullptr;
    RecordStruct->flags = lyo1::StructFlags::NONE;
    state.structs.push_back(RecordStruct);
    state.structcache[RecordStruct->structPath] = RecordStruct;

    tu_uint32 symbol_index = state.symbols.size();
    auto *RecordSymbol = new CoreSymbol();
    RecordSymbol->section = lyo1::DescriptorSection::Struct;
    RecordSymbol->index = struct_index;
    state.symbols.push_back(RecordSymbol);
    TU_ASSERT (!state.symboltable.contains(RecordStruct->structPath));
    state.symboltable[RecordStruct->structPath] = symbol_index;

    return RecordStruct;
}

void
build_core_Record(BuilderState &state, const CoreStruct *RecordStruct)
{
    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(RecordStruct, {}, code);
        state.setStructAllocator(RecordStruct, "RecordAlloc");
    }
}
