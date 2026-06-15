
#include "compile_discard_protocol.h"
#include "prelude_symbols.h"

CoreProtocol *
build_core_DiscardProtocol(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *AnyType = preludeSymbols.AnyExistential->existentialType;
    auto *UndefType = preludeSymbols.UndefExistential->existentialType;

    lyric_common::SymbolPath protocolPath({"DiscardProtocol"});

    auto *DiscardProtocol = state.addProtocol(protocolPath, preludeSymbols.ProtocolExistential,
        AnyType, UndefType, lyo1::PortType::Connect, lyo1::CommunicationType::Send,
        lyo1::ProtocolFlags::NONE);
    return DiscardProtocol;
}
