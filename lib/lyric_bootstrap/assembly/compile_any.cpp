
#include <lyric_common/symbol_url.h>

#include "compile_any.h"

CoreExistential *build_core_Any(BuilderState &state)
{
    uint32_t existential_index = state.existentials.size();

    auto *AnyType = state.addConcreteType(nullptr, lyo1::TypeSection::Existential, existential_index);

    auto *AnyExistential = new CoreExistential();
    AnyExistential->existential_index = existential_index;
    AnyExistential->existentialPath = lyric_common::SymbolPath({"Any"});
    AnyExistential->existentialType = AnyType;
    AnyExistential->superExistential = nullptr;
    AnyExistential->intrinsicMapping = lyo1::IntrinsicType::Invalid;
    AnyExistential->flags = lyo1::ExistentialFlags::Sealed;
    state.existentials.push_back(AnyExistential);
    state.existentialcache[AnyExistential->existentialPath] = AnyExistential;

    auto *AnySymbol = new CoreSymbol();
    AnySymbol->symbolPath = AnyExistential->existentialPath;
    AnySymbol->section = lyo1::DescriptorSection::Existential;
    AnySymbol->index = existential_index;
    TU_ASSERT (!state.symbols.contains(AnySymbol->symbolPath));
    state.symbols[AnySymbol->symbolPath] = AnySymbol;

    return AnyExistential;
}
