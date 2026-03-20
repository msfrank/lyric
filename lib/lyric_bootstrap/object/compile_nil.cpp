
#include <lyric_common/symbol_url.h>

#include "compile_nil.h"

CoreExistential *build_core_Nil(BuilderState &state, const CoreExistential *IntrinsicExistential)
{
    lyric_common::SymbolPath existentialPath({"Nil"});
    auto *NilExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Nil,
        lyo1::ExistentialFlags::Final, IntrinsicExistential);
    return NilExistential;
}
