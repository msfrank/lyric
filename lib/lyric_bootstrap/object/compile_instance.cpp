
#include <lyric_common/symbol_url.h>

#include "compile_instance.h"

CoreExistential *build_core_Instance(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Instance"});

    auto *InstanceExistential = state.addExistential(existentialPath,
        lyo1::IntrinsicType::Instance, lyo1::ExistentialFlags::Final, DescriptorExistential);
    return InstanceExistential;
}
