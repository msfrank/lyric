
#include <lyric_common/symbol_url.h>

#include "compile_concept.h"

CoreExistential *build_core_Concept(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Concept"});

    auto *ConceptExistential = state.addExistential(existentialPath,
        lyo1::IntrinsicType::Concept, lyo1::ExistentialFlags::Final, DescriptorExistential);
    return ConceptExistential;
}
