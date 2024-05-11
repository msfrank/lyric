
#include <lyric_common/symbol_url.h>

#include "compile_class.h"

CoreExistential *build_core_Class(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Class"});

    auto *ClassExistential = state.addExistential(existentialPath,
        lyo1::IntrinsicType::Class, lyo1::ExistentialFlags::Final, DescriptorExistential);
    return ClassExistential;
}