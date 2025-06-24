#include <absl/strings/substitute.h>

#include <tempo_utils/log_stream.h>

#include <lyric_runtime/interpreter_state.h>

#include "iterator_ref.h"

IteratorRef::IteratorRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
}

IteratorRef::~IteratorRef()
{
    TU_LOG_INFO << "free" << IteratorRef::toString();
}

lyric_runtime::DataCell IteratorRef::getField(const lyric_runtime::DataCell &field) const
{
    return lyric_runtime::DataCell();
}

lyric_runtime::DataCell
IteratorRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return lyric_runtime::DataCell();
}

std::string
IteratorRef::toString() const
{
    return absl::Substitute("<$0: IteratorRef $1 vtable=$2>",
        this,
        lyric_runtime::BaseRef::getVirtualTable()->getSymbolUrl().toString(),
        m_vtable);
}

void
IteratorRef::setMembersReachable()
{
}

void
IteratorRef::clearMembersReachable()
{
}

tempo_utils::Status
iterator_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<IteratorRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
iterator_valid(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<lyric_runtime::AbstractRef *>(receiver.data.ref);
    currentCoro->pushData(lyric_runtime::DataCell(instance->iteratorValid()));

    return {};
}

tempo_utils::Status
iterator_next(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();

    TU_ASSERT(frame.numArguments() == 0);

    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<lyric_runtime::AbstractRef *>(receiver.data.ref);

    lyric_runtime::DataCell next;
    if (!instance->iteratorNext(next)) {
        next = lyric_runtime::DataCell();
    }
    currentCoro->pushData(next);

    return {};
}
