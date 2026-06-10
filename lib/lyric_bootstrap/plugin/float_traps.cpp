
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

    double f64;
    TU_ASSERT(receiver.getF64(f64));
    auto result = lyric_runtime::Operand::fromF64(std::ceil(f64));
    return currentCoro->pushData(result);
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

    double f64;
    TU_ASSERT(receiver.getF64(f64));
    auto result = lyric_runtime::Operand::fromF64(std::floor(f64));
    return currentCoro->pushData(result);
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

    double f64;
    TU_ASSERT(receiver.getF64(f64));
    auto result = lyric_runtime::Operand::fromF64(std::trunc(f64));
    return currentCoro->pushData(result);
}
