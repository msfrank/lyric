
#include "compile_undef.h"

CoreExistential *build_core_Undef(BuilderState &state, const CoreExistential *IntrinsicExistential)
{
    lyric_common::SymbolPath existentialPath({"Undef"});
    auto *UndefExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Undef,
        lyo1::ExistentialFlags::Final, IntrinsicExistential);
    return UndefExistential;
}
