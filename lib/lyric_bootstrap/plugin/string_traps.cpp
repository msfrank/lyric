
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

    TU_ASSERT (frame.numArguments() == 1);
    const auto &index = frame.getArgument(0);
    TU_ASSERT(index.type == lyric_runtime::DataCellType::I64);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STRING);
    auto *instance = receiver.data.str;
    currentCoro->pushData(instance->stringAt(index.data.i64));
    return {};
}

tempo_utils::Status
string_compare(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::STRING);
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.type == lyric_runtime::DataCellType::STRING);

    auto *lhs = arg0.data.str;
    auto *rhs = arg1.data.str;
    currentCoro->pushData(lhs->stringCompare(rhs));
    return {};
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STRING);
    auto *instance = receiver.data.str;
    currentCoro->pushData(instance->stringLength());
    return {};
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STRING);
    auto *instance = receiver.data.str;
    std::string utf8;
    instance->utf8Value(utf8);
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STRING);

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::STRING);

    auto rope = receiver.data.str->getStringData().append(arg0.data.str->getStringData());;
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STRING);

    TU_ASSERT(frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::STRING);

    auto rope = receiver.data.str->getStringData().prepend(arg0.data.str->getStringData());;
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STRING);

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::I64);
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.type == lyric_runtime::DataCellType::STRING);

    auto offset = static_cast<tu_int32>(arg0.data.i64);
    auto rope = receiver.data.str->getStringData().insert(offset, arg1.data.str->getStringData());;
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STRING);

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::I64);
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.type == lyric_runtime::DataCellType::I64);

    auto offset = static_cast<tu_int32>(arg0.data.i64);
    auto count = static_cast<tu_int32>(arg1.data.i64);
    auto rope = receiver.data.str->getStringData().remove(offset, count);;
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STRING);

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::I64);
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.type == lyric_runtime::DataCellType::I64);

    auto offset = static_cast<tu_int32>(arg0.data.i64);
    auto count = static_cast<tu_int32>(arg1.data.i64);
    auto rope = receiver.data.str->getStringData().subspan(offset, count);;
    return heapManager->loadStringOntoStack(rope);
}
