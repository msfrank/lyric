#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

#include "record_ref.h"

RecordRef::RecordRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
    m_fields.resize(vtable->getLayoutTotal());
}

RecordRef::~RecordRef()
{
    TU_LOG_VV << "free" << RecordRef::toString();
}

bool
RecordRef::getField(const lyric_runtime::Operand &field, lyric_runtime::Operand &value) const
{
    auto *vtable = getVirtualTable();
    auto *member = vtable->getMember(field);
    if (member == nullptr)
        return false;
    auto offset = member->getLayoutOffset();
    if (m_fields.size() <= offset)
        return false;
    value = m_fields.at(offset);
    return true;
}

bool
RecordRef::setField(
    const lyric_runtime::Operand &field,
    const lyric_runtime::Operand &value,
    lyric_runtime::Operand *prev)
{
    auto *vtable = getVirtualTable();
    auto *member = vtable->getMember(field);
    if (member == nullptr)
        return false;
    auto offset = member->getLayoutOffset();
    if (m_fields.size() <= offset)
        return false;
    if (prev != nullptr) {
        *prev = m_fields.at(offset);
    }
    m_fields[offset] = value;
    return true;
}

std::string
RecordRef::toString() const
{
    return absl::Substitute("<$0: RecordRef $1 vtable=$2>",
        this,
        lyric_runtime::BaseRef::getVirtualTable()->getSymbolUrl().toString(),
        m_vtable);
}

void
RecordRef::setMembersReachable()
{
    for (auto &cell : m_fields) {
        cell.setReachable();
    }
}

void
RecordRef::clearMembersReachable()
{
    for (auto &cell : m_fields) {
        cell.clearReachable();
    }
}

tempo_utils::Status
record_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<RecordRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}
