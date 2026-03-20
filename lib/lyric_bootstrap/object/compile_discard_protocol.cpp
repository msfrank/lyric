
#include "compile_discard_protocol.h"

CoreProtocol *
build_core_DiscardProtocol(
    BuilderState &state,
    const CoreExistential *ProtocolExistential,
    const CoreType *AnyType,
    const CoreType *UndefType)
{
    lyric_common::SymbolPath protocolPath({"DiscardProtocol"});

    auto *DiscardProtocol = state.addProtocol(protocolPath, ProtocolExistential,
        AnyType, UndefType, lyo1::PortType::Connect, lyo1::CommunicationType::Send,
        lyo1::ProtocolFlags::NONE);
    return DiscardProtocol;
}
