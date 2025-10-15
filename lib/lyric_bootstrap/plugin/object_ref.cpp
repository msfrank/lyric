#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

#include "object_ref.h"

ObjectRef::ObjectRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
    m_fields.resize(vtable->getLayoutTotal());
}

ObjectRef::~ObjectRef()
{
    TU_LOG_VV << "free" << ObjectRef::toString();
}

bool
ObjectRef::getField(const lyric_runtime::DataCell &field, lyric_runtime::DataCell &value) const
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
ObjectRef::setField(
    const lyric_runtime::DataCell &field,
    const lyric_runtime::DataCell &value,
    lyric_runtime::DataCell *prev)
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
ObjectRef::toString() const
{
    return absl::Substitute("<$0: ObjectRef $1 vtable=$2>",
        this,
        lyric_runtime::BaseRef::getVirtualTable()->getSymbolUrl().toString(),
        m_vtable);
}

void
ObjectRef::setMembersReachable()
{
    for (auto &cell : m_fields) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->setReachable();
        }
    }
}

void
ObjectRef::clearMembersReachable()
{
    for (auto &cell : m_fields) {
        if (cell.type == lyric_runtime::DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->clearReachable();
        }
    }
}

tempo_utils::Status
object_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<ObjectRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}
