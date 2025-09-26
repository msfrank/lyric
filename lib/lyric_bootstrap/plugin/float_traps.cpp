
#include "float_traps.h"

tempo_utils::Status
float_ceil(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::DBL);

    lyric_runtime::DataCell floor{std::ceil(receiver.data.dbl)};
    currentCoro->pushData(floor);
    return {};
}

tempo_utils::Status
float_floor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::DBL);

    lyric_runtime::DataCell floor{std::floor(receiver.data.dbl)};
    currentCoro->pushData(floor);
    return {};
}

tempo_utils::Status
float_trunc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::DBL);

    lyric_runtime::DataCell floor{std::trunc(receiver.data.dbl)};
    currentCoro->pushData(floor);
    return {};
}
