
#include "compile_undef.h"

CoreExistential *build_core_Undef(BuilderState &state)
{
    uint32_t existential_index = state.existentials.size();

    auto *UndefType = state.addConcreteType(nullptr, lyo1::TypeSection::Existential, existential_index);

    auto *UndefExistential = new CoreExistential();
    UndefExistential->existential_index = existential_index;
    UndefExistential->existentialPath = lyric_common::SymbolPath({"Undef"});
    UndefExistential->existentialType = UndefType;
    UndefExistential->superExistential = nullptr;
    UndefExistential->intrinsicMapping = lyo1::IntrinsicType::Undef;
    UndefExistential->flags = lyo1::ExistentialFlags::Final;
    state.existentials.push_back(UndefExistential);
    state.existentialcache[UndefExistential->existentialPath] = UndefExistential;

    tu_uint32 symbol_index = state.symbols.size();
    auto *UndefSymbol = new CoreSymbol();
    UndefSymbol->section = lyo1::DescriptorSection::Existential;
    UndefSymbol->index = existential_index;
    state.symbols.push_back(UndefSymbol);
    TU_ASSERT (!state.symboltable.contains(UndefExistential->existentialPath));
    state.symboltable[UndefExistential->existentialPath] = symbol_index;

    return UndefExistential;
}
