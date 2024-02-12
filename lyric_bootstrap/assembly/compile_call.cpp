
#include <lyric_common/symbol_url.h>

#include "compile_call.h"

CoreExistential *build_core_Call(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Call"});
    auto *CallExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Call,
        lyo1::ExistentialFlags::Final, DescriptorExistential);
    return CallExistential;
}