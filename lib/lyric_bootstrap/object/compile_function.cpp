
#include "compile_function.h"

CoreClass *
declare_core_FunctionN(BuilderState &state, int arity, const CoreClass *ObjectClass)
{
    lyric_common::SymbolPath classPath({absl::StrCat("Function", arity)});

    std::vector<CorePlaceholder> placeholders;
    placeholders.push_back({"R", lyo1::PlaceholderVariance::Covariant});
    for (int i = 0; i < arity; i++) {
        auto name = absl::StrCat("P", i);
        placeholders.push_back({name, lyo1::PlaceholderVariance::Contravariant});
    }

    auto *FunctionNTemplate = state.addTemplate(classPath, placeholders);

    auto *FunctionNClass = state.addGenericClass(classPath,
        FunctionNTemplate, lyo1::ClassFlags::Final,
        ObjectClass);

    TU_ASSERT (!state.functionclasspaths.contains(arity));
    state.functionclasspaths[arity] = FunctionNClass->classPath;

    return FunctionNClass;
}

void
build_core_FunctionN(BuilderState &state, int arity, const CoreClass *FunctionNClass, const CoreType *CallType)
{
    auto classPath = FunctionNClass->classPath;

    auto *FunctionNTemplate = FunctionNClass->classTemplate;

    std::vector<CoreParam> applyParams;
    for (int i = 0; i < arity; i++) {
        auto offset = i;
        auto name = absl::StrCat("p", offset);
        auto *PType = FunctionNTemplate->types.at(FunctionNTemplate->names.at(offset));
        applyParams.push_back(make_list_param(name, PType));
    }

    auto *RType = FunctionNTemplate->types.at("R");

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "ClosureCtor");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(FunctionNClass,
            {
                make_list_param("$call", CallType),
            },
            code);
        state.setClassAllocator(FunctionNClass, "ClosureAlloc");
    }
    {
        lyric_object::BytecodeBuilder code;
        for (tu_uint32 i = 0; i < applyParams.size(); i++) {
            code.loadArgument(i);
        }
        state.writeTrap(code, "ClosureApply");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);

        state.addClassMethod("apply", FunctionNClass,
            lyo1::CallFlags::GlobalVisibility, applyParams,
            code, RType);
    }
}
