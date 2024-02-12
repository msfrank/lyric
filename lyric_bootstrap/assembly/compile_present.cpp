
#include "compile_present.h"

CoreExistential *build_core_Present(BuilderState &state, const CoreExistential *DataExistential)
{
    lyric_common::SymbolPath existentialPath({"Present"});
    auto *PresentExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Present,
        lyo1::ExistentialFlags::Final, DataExistential);
    return PresentExistential;
}
