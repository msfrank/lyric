
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/log_stream.h>

#include "string_traps.h"

tempo_utils::Status
string_at(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT (frame.numArguments() == 1);
    const auto &index = frame.getArgument(0);
    TU_ASSERT(index.type == lyric_runtime::DataCellType::I64);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STRING);
    auto *instance = receiver.data.str;
    currentCoro->pushData(instance->stringAt(index.data.i64));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
string_compare(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::STRING);
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.type == lyric_runtime::DataCellType::STRING);

    auto *lhs = arg0.data.str;
    auto *rhs = arg1.data.str;
    currentCoro->pushData(lhs->stringCompare(rhs));
    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
string_length(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STRING);
    auto *instance = receiver.data.str;
    currentCoro->pushData(instance->stringLength());
    return lyric_runtime::InterpreterStatus::ok();
}
