#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

#include "status_ref.h"

StatusRef::StatusRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
    m_fields.resize(vtable->getLayoutTotal());
}

StatusRef::~StatusRef()
{
    TU_LOG_INFO << "free" << StatusRef::toString();
}

lyric_runtime::DataCell
StatusRef::getField(const lyric_runtime::DataCell &field) const
{
    auto *vtable = getVirtualTable();
    auto *member = vtable->getMember(field);
    if (member == nullptr)
        return {};
    auto offset = member->getLayoutOffset();
    if (m_fields.size() <= offset)
        return {};
    return m_fields.at(offset);
}

lyric_runtime::DataCell
StatusRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    auto *vtable = getVirtualTable();
    auto *member = vtable->getMember(field);
    if (member == nullptr)
        return {};
    auto offset = member->getLayoutOffset();
    if (m_fields.size() <= offset)
        return {};
    auto prev = m_fields.at(offset);
    m_fields[offset] = value;
    return prev;
}

std::string
StatusRef::toString() const
{
    return absl::Substitute("<$0: StatusRef $1 vtable=$2>",
        this,
        lyric_runtime::BaseRef::getVirtualTable()->getSymbolUrl().toString(),
        m_vtable);
}

void
StatusRef::setMembersReachable()
{
    for (auto &cell : m_fields) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->setReachable();
        }
    }
}

void
StatusRef::clearMembersReachable()
{
    for (auto &cell : m_fields) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->clearReachable();
        }
    }
}

tempo_utils::Status
status_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<StatusRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}
