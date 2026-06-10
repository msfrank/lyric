
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/protocol_ref.h>
#include <tempo_utils/log_stream.h>

#include "protocol_traps.h"

tempo_utils::Status
protocol_is_acceptor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::ProtocolRef *protocol;
    TU_ASSERT(receiver.getProtocol(protocol));

    TU_ASSERT (frame.numArguments() == 0);
    auto result = protocol->protocolIsAcceptor();
    return currentCoro->pushData(result);
}

tempo_utils::Status
protocol_is_connector(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::ProtocolRef *protocol;
    TU_ASSERT(receiver.getProtocol(protocol));

    TU_ASSERT (frame.numArguments() == 0);
    auto result = protocol->protocolIsConnector();
    return currentCoro->pushData(result);
}

tempo_utils::Status
protocol_can_send(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::ProtocolRef *protocol;
    TU_ASSERT(receiver.getProtocol(protocol));

    TU_ASSERT (frame.numArguments() == 0);
    auto result = protocol->protocolCanSend();
    return currentCoro->pushData(result);
}

tempo_utils::Status
protocol_can_receive(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::ProtocolRef *protocol;
    TU_ASSERT(receiver.getProtocol(protocol));

    TU_ASSERT (frame.numArguments() == 0);
    auto result = protocol->protocolCanReceive();
    return currentCoro->pushData(result);
}
