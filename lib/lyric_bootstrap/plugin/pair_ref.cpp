
#include <absl/strings/substitute.h>

#include "pair_ref.h"
#include "map_key.h"

PairRef::PairRef(const lyric_runtime::VirtualTable *vtable)
    : lyric_runtime::BaseRef(vtable)
{
    TU_ASSERT (vtable != nullptr);
}

PairRef::~PairRef()
{
    TU_LOG_VV << "free " << PairRef::toString();
}

lyric_runtime::DataCell
PairRef::getField(const lyric_runtime::DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
PairRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return {};
}

bool
PairRef::hashValue(absl::HashState state)
{
    absl::HashState::combine(std::move(state), MapKey{m_first}, MapKey{m_second});
    return true;
}

std::string
PairRef::toString() const
{
    return absl::Substitute("<$0: PairRef>", this);
}

void
PairRef::setPair(const lyric_runtime::DataCell &first, const lyric_runtime::DataCell &second)
{
    TU_ASSERT (first.isValid());
    TU_ASSERT (second.isValid());
    TU_ASSERT (!m_first.isValid());
    TU_ASSERT (!m_second.isValid());
    m_first = first;
    m_second = second;
}

lyric_runtime::DataCell
PairRef::pairFirst() const
{
    return m_first;
}

lyric_runtime::DataCell
PairRef::pairSecond() const
{
    return m_second;
}

void
PairRef::setMembersReachable()
{
    if (m_first.type == lyric_runtime::DataCellType::REF)
        m_first.data.ref->setReachable();
    if (m_second.type == lyric_runtime::DataCellType::REF)
        m_second.data.ref->setReachable();
}

void
PairRef::clearMembersReachable()
{
    if (m_first.type == lyric_runtime::DataCellType::REF)
        m_first.data.ref->clearReachable();
    if (m_second.type == lyric_runtime::DataCellType::REF)
        m_second.data.ref->clearReachable();
}

tempo_utils::Status
pair_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<PairRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
pair_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *pair = static_cast<PairRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    const auto &arg1 = frame.getArgument(1);
    pair->setPair(arg0, arg1);

    return {};
}

tempo_utils::Status
pair_first(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    TU_ASSERT (frame.numArguments() == 0);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *pair = static_cast<PairRef *>(receiver.data.ref);

    currentCoro->pushData(pair->pairFirst());
    return {};
}

tempo_utils::Status
pair_second(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    TU_ASSERT (frame.numArguments() == 0);
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *pair = static_cast<PairRef *>(receiver.data.ref);

    currentCoro->pushData(pair->pairSecond());
    return {};
}