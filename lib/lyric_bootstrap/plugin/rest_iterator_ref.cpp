
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
    m_size = rest->numRest();
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
RestIterator::iteratorNext(lyric_runtime::Operand &cell)
{
    if (m_curr < m_size) {
        cell = m_rest->restAt(m_curr++);
        return true;
    } else {
        cell = lyric_runtime::Operand();
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
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));

    TU_ASSERT(frame.numArguments() == 0);
    return currentCoro->pushData(lyric_runtime::Operand::fromBool(ref->iteratorValid()));
}

tempo_utils::Status
rest_iterator_next(
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
