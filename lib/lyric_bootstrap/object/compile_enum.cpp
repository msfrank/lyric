
#include <lyric_common/symbol_url.h>

#include "compile_enum.h"

CoreExistential *build_core_Enum(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Enum"});

    auto *EnumExistential = state.addExistential(existentialPath,
        lyo1::IntrinsicType::Enum, lyo1::ExistentialFlags::Final, DescriptorExistential);
    return EnumExistential;
}
