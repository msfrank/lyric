
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
    return currentCoro->pushData(status);
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
    lyric_runtime::StatusRef *status;
    TU_ASSERT(receiver.getStatus(status));

    tu_int64 code;
    lyric_runtime::StringRef *message;
    TU_ASSERT (frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.getI64(code));
    auto statusCode = static_cast<tempo_utils::StatusCode>(code);
    status->setStatusCode(statusCode);
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT (arg1.getString(message));
    status->setMessage(arg1);

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
    lyric_runtime::StatusRef *status;
    TU_ASSERT(receiver.getStatus(status));

    return currentCoro->pushData(status->getStatusCode());
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
    lyric_runtime::StatusRef *status;
    TU_ASSERT(receiver.getStatus(status));

    return currentCoro->pushData(status->getMessage());
}
