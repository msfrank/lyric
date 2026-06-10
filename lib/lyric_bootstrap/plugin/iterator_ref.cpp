#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

#include "iterator_ref.h"

IteratorRef::IteratorRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

IteratorRef::~IteratorRef()
{
    TU_LOG_VV << "free" << IteratorRef::toString();
}

std::string
IteratorRef::toString() const
{
    return absl::Substitute("<$0: IteratorRef $1 vtable=$2>",
        this,
        lyric_runtime::BaseRef::getVirtualTable()->getSymbolUrl().toString(),
        m_vtable);
}

tempo_utils::Status
iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<IteratorRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));

    TU_ASSERT(frame.numArguments() == 0);
    return currentCoro->pushData(lyric_runtime::Operand::fromBool(ref->iteratorValid()));
}

tempo_utils::Status
iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));

    TU_ASSERT(frame.numArguments() == 0);

    lyric_runtime::Operand next;
    if (!ref->iteratorNext(next)) {
        next = lyric_runtime::Operand();
    }
    currentCoro->pushData(next);

    return {};
}
