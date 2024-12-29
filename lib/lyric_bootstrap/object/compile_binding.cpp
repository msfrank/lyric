
#include <lyric_common/symbol_url.h>

#include "compile_binding.h"

CoreExistential *build_core_Binding(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Binding"});

    auto *BindingExistential = state.addExistential(existentialPath,
        lyo1::IntrinsicType::Binding, lyo1::ExistentialFlags::Final, DescriptorExistential);
    return BindingExistential;
}