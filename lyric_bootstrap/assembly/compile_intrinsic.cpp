
#include <lyric_common/symbol_url.h>

#include "compile_intrinsic.h"

CoreExistential *build_core_Intrinsic(BuilderState &state, const CoreExistential *AnyExistential)
{
    lyric_common::SymbolPath existentialPath({"Intrinsic"});
    auto *IntrinsicExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Invalid,
        lyo1::ExistentialFlags::Sealed, AnyExistential);
    return IntrinsicExistential;
}
