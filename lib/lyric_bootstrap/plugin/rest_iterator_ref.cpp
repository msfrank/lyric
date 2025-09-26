
#include "rest_iterator_ref.h"

#include <absl/strings/substitute.h>

RestIterator::RestIterator(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_curr(0),
      m_size(0),
      m_rest(nullptr)
{
}

RestIterator::RestIterator(const lyric_runtime::VirtualTable *vtable, lyric_runtime::RestRef *rest)
    : BaseRef(vtable),
      m_curr(0),
      m_size(0),
      m_rest(rest)
{
    TU_ASSERT (m_rest != nullptr);
    auto size = m_rest->restLength();
    TU_ASSERT (size.type == lyric_runtime::DataCellType::I64);
    m_size = size.data.i64;
}

lyric_runtime::DataCell
RestIterator::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
RestIterator::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

std::string
RestIterator::toString() const
{
    return absl::Substitute("<$0: RestIterator>", this);
}

bool
RestIterator::iteratorValid()
{
    return m_curr < m_size;
}

bool
RestIterator::iteratorNext(lyric_runtime::DataCell &cell)
{
    if (m_curr < m_size) {
        cell = m_rest->restAt(m_curr++);
        return true;
    } else {
        cell = lyric_runtime::DataCell();
        return false;
    }
}

void
RestIterator::setMembersReachable()
{
    m_rest->setReachable();
}

void
RestIterator::clearMembersReachable()
{
    m_rest->clearReachable();
}

tempo_utils::Status
rest_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<RestIterator>(vtable);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
rest_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
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
rest_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
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
