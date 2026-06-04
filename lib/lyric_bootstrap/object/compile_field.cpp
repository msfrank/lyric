
#include <lyric_common/symbol_url.h>

#include "compile_field.h"

CoreExistential *build_core_Field(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Field"});

    auto *FieldExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, DescriptorExistential);
    return FieldExistential;
}