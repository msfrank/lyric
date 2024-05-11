
#include <lyric_common/symbol_url.h>

#include "compile_descriptor.h"

CoreExistential *build_core_Descriptor(BuilderState &state, const CoreExistential *AnyExistential)
{
    lyric_common::SymbolPath existentialPath({"Descriptor"});
    auto *DescriptorExistential = state.addExistential(existentialPath, lyo1::IntrinsicType::Invalid,
        lyo1::ExistentialFlags::Sealed, AnyExistential);
    return DescriptorExistential;
}
