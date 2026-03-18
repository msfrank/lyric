
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

    TU_ASSERT (frame.numArguments() == 0);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::PROTOCOL);
    auto *instance = receiver.data.protocol;
    auto result = instance->protocolIsAcceptor();
    currentCoro->pushData(result);
    return {};
}

tempo_utils::Status
protocol_is_connector(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT (frame.numArguments() == 0);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::PROTOCOL);
    auto *instance = receiver.data.protocol;
    auto result = instance->protocolIsConnector();
    currentCoro->pushData(result);
    return {};
}

tempo_utils::Status
protocol_can_send(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT (frame.numArguments() == 0);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::PROTOCOL);
    auto *instance = receiver.data.protocol;
    auto result = instance->protocolCanSend();
    currentCoro->pushData(result);
    return {};
}

tempo_utils::Status
protocol_can_receive(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT (frame.numArguments() == 0);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::PROTOCOL);
    auto *instance = receiver.data.protocol;
    auto result = instance->protocolCanReceive();
    currentCoro->pushData(result);
    return {};
}
