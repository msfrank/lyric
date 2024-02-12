
#include <lyric_common/symbol_url.h>

#include "compile_any.h"

CoreExistential *build_core_Code(BuilderState &state, const CoreExistential *AnyExistential)
{
    lyric_common::SymbolPath existentialPath({"Code"});
    auto *CodeExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Invalid,
        lyo1::ExistentialFlags::Sealed, AnyExistential);
    return CodeExistential;
}
