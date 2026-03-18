
#include "compile_protocol.h"

CoreExistential *
declare_core_Protocol(BuilderState &state, const CoreExistential *DescriptorExistential)
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

void
build_core_Protocol(
    BuilderState &state,
    const CoreExistential *ProtocolExistential,
    const CoreType *BoolType)
{
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "ProtocolIsAcceptor");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("IsAcceptor",
            ProtocolExistential,
            lyo1::CallFlags::NONE,
            {},
            code,
            BoolType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "ProtocolIsConnector");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("IsConnector",
            ProtocolExistential,
            lyo1::CallFlags::NONE,
            {},
            code,
            BoolType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "ProtocolCanSend");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("CanSend",
            ProtocolExistential,
            lyo1::CallFlags::NONE,
            {},
            code,
            BoolType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "ProtocolCanReceive");
        TU_RAISE_IF_NOT_OK(code.writeOpcode(lyric_object::Opcode::OP_RETURN));
        state.addExistentialMethod("CanReceive",
            ProtocolExistential,
            lyo1::CallFlags::NONE,
            {},
            code,
            BoolType);
    }
}