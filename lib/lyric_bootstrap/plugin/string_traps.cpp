
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/string_ref.h>
#include <tempo_utils/log_stream.h>

#include "string_traps.h"

tempo_utils::Status
string_at(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::StringRef *string;
    TU_ASSERT(receiver.getString(string));

    tu_int64 index;
    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getI64(index));

    return currentCoro->pushData(string->stringAt(index));
}

tempo_utils::Status
string_compare(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    lyric_runtime::StringRef *lhs, *rhs;
    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getString(lhs));
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.getString(rhs));

    return currentCoro->pushData(lhs->stringCompare(rhs));
}

tempo_utils::Status
string_length(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::StringRef *string;
    TU_ASSERT(receiver.getString(string));

    return currentCoro->pushData(string->stringLength());
}

tempo_utils::Status
string_to_bytes(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *heapManager = state->heapManager();
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::StringRef *string;
    TU_ASSERT(receiver.getString(string));

    std::string utf8;
    string->utf8Value(utf8);
    auto bytes = std::span((const tu_uint8 *) utf8.data(), utf8.size());
    return heapManager->loadBytesOntoStack(bytes);
}

tempo_utils::Status
string_append(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::StringRef *string;
    TU_ASSERT(receiver.getString(string));

    lyric_runtime::StringRef *other;
    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getString(other));

    auto rope = string->getStringData().append(other->getStringData());;
    return heapManager->loadStringOntoStack(rope);
}

tempo_utils::Status
string_prepend(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::StringRef *string;
    TU_ASSERT(receiver.getString(string));

    lyric_runtime::StringRef *other;
    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getString(other));

    auto rope = string->getStringData().prepend(other->getStringData());;
    return heapManager->loadStringOntoStack(rope);
}

tempo_utils::Status
string_insert(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::StringRef *string;
    TU_ASSERT(receiver.getString(string));

    tu_int64 offset;
    lyric_runtime::StringRef *other;
    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getI64(offset));
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.getString(other));

    auto rope = string->getStringData().insert(offset, other->getStringData());;
    return heapManager->loadStringOntoStack(rope);
}

tempo_utils::Status
string_remove(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::StringRef *string;
    TU_ASSERT(receiver.getString(string));

    tu_int64 offset, count;
    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getI64(offset));
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.getI64(count));

    auto rope = string->getStringData().remove(offset, count);;
    return heapManager->loadStringOntoStack(rope);
}

tempo_utils::Status
string_substring(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::StringRef *string;
    TU_ASSERT(receiver.getString(string));

    tu_int64 offset, count;
    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getI64(offset));
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.getI64(count));

    auto rope = string->getStringData().subspan(offset, count);;
    return heapManager->loadStringOntoStack(rope);
}
