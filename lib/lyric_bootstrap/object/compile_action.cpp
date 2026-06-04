
#include <lyric_common/symbol_url.h>

#include "compile_action.h"

CoreExistential *build_core_Action(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Action"});

    auto *ActionExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, DescriptorExistential);
    return ActionExistential;
}