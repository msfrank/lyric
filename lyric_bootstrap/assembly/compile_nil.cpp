
#include <lyric_common/symbol_url.h>

#include "compile_nil.h"

CoreExistential *build_core_Nil(BuilderState &state)
{
    uint32_t existential_index = state.existentials.size();

    auto *NilType = state.addConcreteType(nullptr, lyo1::TypeSection::Existential, existential_index);

    auto *NilExistential = new CoreExistential();
    NilExistential->existential_index = existential_index;
    NilExistential->existentialPath = lyric_common::SymbolPath({"Nil"});
    NilExistential->existentialType = NilType;
    NilExistential->superExistential = nullptr;
    NilExistential->intrinsicMapping = lyo1::IntrinsicType::Nil;
    NilExistential->flags = lyo1::ExistentialFlags::Final;
    state.existentials.push_back(NilExistential);
    state.existentialcache[NilExistential->existentialPath] = NilExistential;

    auto *NilSymbol = new CoreSymbol();
    NilSymbol->symbolPath = NilExistential->existentialPath;
    NilSymbol->section = lyo1::DescriptorSection::Existential;
    NilSymbol->index = existential_index;
    TU_ASSERT (!state.symbols.contains(NilSymbol->symbolPath));
    state.symbols[NilSymbol->symbolPath] = NilSymbol;

    return NilExistential;
}
