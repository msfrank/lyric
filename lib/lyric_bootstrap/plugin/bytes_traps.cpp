
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/bytes_ref.h>
#include <tempo_utils/log_stream.h>

#include "bytes_traps.h"

tempo_utils::Status
bytes_at(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BytesRef *bytes;
    TU_ASSERT (receiver.getBytes(bytes));

    tu_int64 index;
    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.getI64(index));

    return currentCoro->pushData(bytes->byteAt(index));
}

tempo_utils::Status
bytes_compare(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    lyric_runtime::BytesRef *lhs, *rhs;
    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.getBytes(lhs));
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT (arg1.getBytes(rhs));

    return currentCoro->pushData(lhs->bytesCompare(rhs));
}

tempo_utils::Status
bytes_length(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BytesRef *bytes;
    TU_ASSERT (receiver.getBytes(bytes));

    return currentCoro->pushData(bytes->bytesLength());
}

tempo_utils::Status
bytes_to_string(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *heapManager = state->heapManager();

    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BytesRef *bytes;
    TU_ASSERT (receiver.getBytes(bytes));

    std::string utf8;
    bytes->utf8Value(utf8);
    return heapManager->loadStringOntoStack(utf8);
}

tempo_utils::Status
bytes_append(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BytesRef *bytes;
    TU_ASSERT (receiver.getBytes(bytes));

    lyric_runtime::BytesRef *other;
    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getBytes(other));

    auto rope = bytes->getBytesData().append(other->getBytesData());;
    return heapManager->loadBytesOntoStack(rope);
}

tempo_utils::Status
bytes_prepend(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BytesRef *bytes;
    TU_ASSERT (receiver.getBytes(bytes));

    lyric_runtime::BytesRef *other;
    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getBytes(other));

    auto rope = bytes->getBytesData().prepend(other->getBytesData());;
    return heapManager->loadBytesOntoStack(rope);
}

tempo_utils::Status
bytes_insert(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BytesRef *bytes;
    TU_ASSERT (receiver.getBytes(bytes));

    tu_int64 offset;
    lyric_runtime::BytesRef *other;
    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getI64(offset));
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.getBytes(other));

    auto rope = bytes->getBytesData().insert(offset, other->getBytesData());;
    return heapManager->loadBytesOntoStack(rope);
}

tempo_utils::Status
bytes_remove(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BytesRef *bytes;
    TU_ASSERT (receiver.getBytes(bytes));

    tu_int64 offset, count;
    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getI64(offset));
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.getI64(count));

    auto rope = bytes->getBytesData().remove(offset, count);;
    return heapManager->loadBytesOntoStack(rope);
}

tempo_utils::Status
bytes_subspan(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BytesRef *bytes;
    TU_ASSERT (receiver.getBytes(bytes));

    tu_int64 offset, count;
    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getI64(offset));
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.getI64(count));

    auto rope = bytes->getBytesData().subspan(offset, count);;
    return heapManager->loadBytesOntoStack(rope);
}
