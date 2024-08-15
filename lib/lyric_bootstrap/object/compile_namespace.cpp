
#include "compile_namespace.h"

CoreExistential *build_core_Namespace(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Namespace"});
    auto *NamespaceExistential = state.addExistential(existentialPath,
        lyo1::IntrinsicType::Namespace, lyo1::ExistentialFlags::Final, DescriptorExistential);
    return NamespaceExistential;
}