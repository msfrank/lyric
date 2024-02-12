
#include "compile_iterator.h"

CoreClass *
build_core_Iterator(BuilderState &state, const CoreClass *ObjectClass, const CoreType *BoolType)
{
    lyric_common::SymbolPath classPath({"Iterator"});

    auto *IteratorTemplate = state.addTemplate(
        classPath,
        {
            {"T", lyo1::PlaceholderVariance::Invariant},
        });

    auto *TType = IteratorTemplate->types["T"];

    auto *IteratorClass = state.addGenericClass(classPath, IteratorTemplate,
        lyo1::ClassFlags::NONE, ObjectClass);

    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(IteratorClass, {}, code);
        state.setClassAllocator(IteratorClass, lyric_bootstrap::internal::BootstrapTrap::ITERATOR_ALLOC);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::ITERATOR_VALID));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassMethod("valid", IteratorClass,
            lyo1::CallFlags::GlobalVisibility, {}, {},
            code, BoolType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::ITERATOR_NEXT));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassMethod("next", IteratorClass,
            lyo1::CallFlags::GlobalVisibility, {}, {},
            code, TType);
    }

    return IteratorClass;
}
