#include "compile_class.h"

CoreExistential *build_core_Struct(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Struct"});

    auto *StructExistential = state.addExistential(existentialPath,
        lyo1::IntrinsicType::Struct, lyo1::ExistentialFlags::Final, DescriptorExistential);
    return StructExistential;
}
