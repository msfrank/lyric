
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/bytes_ref.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/log_stream.h>

#include "bytes_traps.h"

tempo_utils::Status
bytes_at(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT (frame.numArguments() == 1);
    const auto &index = frame.getArgument(0);
    TU_ASSERT(index.type == lyric_runtime::DataCellType::I64);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::BYTES);
    auto *instance = receiver.data.bytes;
    currentCoro->pushData(instance->byteAt(index.data.i64));
    return {};
}

tempo_utils::Status
bytes_compare(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::BYTES);
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.type == lyric_runtime::DataCellType::BYTES);

    auto *lhs = arg0.data.bytes;
    auto *rhs = arg1.data.bytes;
    currentCoro->pushData(lhs->bytesCompare(rhs));
    return {};
}

tempo_utils::Status
bytes_length(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::BYTES);
    auto *instance = receiver.data.bytes;
    currentCoro->pushData(instance->bytesLength());
    return {};
}
