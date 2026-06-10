
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/rest_ref.h>

#include "rest_iterator_ref.h"
#include "rest_traps.h"

tempo_utils::Status
rest_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::RestRef *rest;
    TU_ASSERT(receiver.getRest(rest));

    return currentCoro->pushData(rest->restLength());
}

tempo_utils::Status
rest_get(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::RestRef *rest;
    TU_ASSERT(receiver.getRest(rest));

    tu_int64 offset;
    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getI64(offset));

    return currentCoro->pushData(rest->restAt(offset));
}

tempo_utils::Status
rest_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::RestRef *rest;
    TU_ASSERT(receiver.getRest(rest));

    lyric_runtime::Operand cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));

    lyric_runtime::InterpreterStatus status;
    vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    if (vtable == nullptr)
        return status;

    auto ref = state->heapManager()->allocateRef<RestIterator>(vtable, rest);
    return currentCoro->pushData(ref);
}
