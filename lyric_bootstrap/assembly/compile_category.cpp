
#include <lyric_common/symbol_url.h>

#include "compile_category.h"

CoreEnum *
declare_core_Category(BuilderState &state, const CoreExistential *AnyExistential)
{
    uint32_t type_index = state.types.size();
    uint32_t enum_index = state.enums.size();

    auto *CategoryType = new CoreType();
    CategoryType->type_index = type_index;
    CategoryType->typeAssignable = lyo1::Assignable::ConcreteAssignable;
    CategoryType->concreteSection = lyo1::TypeSection::Enum;
    CategoryType->concreteDescriptor = enum_index;
    CategoryType->superType = AnyExistential->existentialType;
    state.types.push_back(CategoryType);

    auto *CategoryEnum = new CoreEnum();
    CategoryEnum->enum_index = enum_index;
    CategoryEnum->enumPath = lyric_common::SymbolPath({"Category"});
    CategoryEnum->enumType = CategoryType;
    CategoryEnum->superEnum = nullptr;
    CategoryEnum->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    CategoryEnum->enumCtor = nullptr;
    CategoryEnum->flags = lyo1::EnumFlags::NONE;
    state.enums.push_back(CategoryEnum);
    state.enumcache[CategoryEnum->enumPath] = CategoryEnum;

    auto *CategorySymbol = new CoreSymbol();
    CategorySymbol->symbolPath = CategoryEnum->enumPath;
    CategorySymbol->section = lyo1::DescriptorSection::Enum;
    CategorySymbol->index = enum_index;
    TU_ASSERT (!state.symbols.contains(CategorySymbol->symbolPath));
    state.symbols[CategorySymbol->symbolPath] = CategorySymbol;

    return CategoryEnum;
}

void
build_core_Category(BuilderState &state, const CoreEnum *CategoryEnum)
{
    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addEnumCtor(CategoryEnum, {}, code);
        state.setEnumAllocator(CategoryEnum, lyric_bootstrap::internal::BootstrapTrap::CATEGORY_ALLOC);
    }
}
