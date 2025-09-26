
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REST);
    auto *instance = receiver.data.rest;
    currentCoro->pushData(instance->restLength());
    return {};
}

tempo_utils::Status
rest_get(
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
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REST);
    auto *instance = receiver.data.rest;
    currentCoro->pushData(instance->restAt(index.data.i64));
    return {};
}

tempo_utils::Status
rest_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    lyric_runtime::DataCell cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));
    TU_ASSERT(cell.type == lyric_runtime::DataCellType::CLASS);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REST);
    auto *instance = receiver.data.rest;

    lyric_runtime::InterpreterStatus status;
    vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    if (vtable == nullptr)
        return status;

    auto ref = state->heapManager()->allocateRef<RestIterator>(vtable, instance);
    currentCoro->pushData(ref);

    return {};
}
