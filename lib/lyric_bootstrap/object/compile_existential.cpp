
#include <lyric_common/symbol_url.h>

#include "compile_binding.h"

CoreExistential *build_core_Existential(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Existential"});

    auto *ExistentialExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, DescriptorExistential);
    return ExistentialExistential;
}