
#include <lyric_runtime/heap_manager.h>
#include <lyric_runtime/status_ref.h>

#include "status_traps.h"

tempo_utils::Status
status_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();
    auto *heapManager = state->heapManager();

    auto status = heapManager->allocateStatus(vtable);
    currentCoro->pushData(status);

    return {};
}

tempo_utils::Status
status_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STATUS);
    auto *instance = receiver.data.status;

    TU_ASSERT (frame.numArguments() > 0);

    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.type == lyric_runtime::DataCellType::I64);
    auto statusCode = static_cast<tempo_utils::StatusCode>(arg0.data.i64);
    instance->setStatusCode(statusCode);

    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT (arg1.type == lyric_runtime::DataCellType::STRING);
    instance->setMessage(arg1);

    return {};
}

tempo_utils::Status
status_get_code(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STATUS);
    auto *instance = receiver.data.status;
    currentCoro->pushData(instance->getStatusCode());

    return {};
}

tempo_utils::Status
status_get_message(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::STATUS);
    auto *instance = receiver.data.status;
    currentCoro->pushData(instance->getMessage());

    return {};
}
