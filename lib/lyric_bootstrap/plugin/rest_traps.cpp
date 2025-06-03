
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/string_ref.h>

#include "rest_traps.h"

#include "lyric_runtime/rest_ref.h"

tempo_utils::Status
rest_num_args(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REST);
    auto *instance = receiver.data.rest;
    currentCoro->pushData(instance->restLength());
    return {};
}

tempo_utils::Status
rest_get_arg(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT (frame.numArguments() == 1);
    const auto &index = frame.getArgument(0);
    TU_ASSERT(index.type == lyric_runtime::DataCellType::I64);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REST);
    auto *instance = receiver.data.rest;
    currentCoro->pushData(instance->restAt(index.data.i64));
    return {};
}
