
#include "compile_protocol.h"

CoreExistential *build_core_Protocol(BuilderState &state, const CoreExistential *DescriptorExistential)
{
    lyric_common::SymbolPath existentialPath({"Protocol"});

    std::vector<CorePlaceholder> placeholders;
    placeholders.push_back({"S", lyo1::PlaceholderVariance::Invariant});
    placeholders.push_back({"R", lyo1::PlaceholderVariance::Invariant});
    auto *ProtocolTemplate = state.addTemplate(existentialPath, placeholders);

    auto *ProtocolExistential = state.addGenericExistential(existentialPath, ProtocolTemplate,
        lyo1::IntrinsicType::Protocol, lyo1::ExistentialFlags::NONE, DescriptorExistential);
    return ProtocolExistential;
}