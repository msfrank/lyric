#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

#include "category_ref.h"

CategoryRef::CategoryRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
    m_fields.resize(vtable->getLayoutTotal());
}

CategoryRef::~CategoryRef()
{
    TU_LOG_VV << "free" << CategoryRef::toString();
}

bool
CategoryRef::getField(const lyric_runtime::Operand &field, lyric_runtime::Operand &value) const
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
CategoryRef::setField(
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
CategoryRef::toString() const
{
    return absl::Substitute("<$0: CategoryRef $1 vtable=$2>",
        this,
        lyric_runtime::BaseRef::getVirtualTable()->getSymbolUrl().toString(),
        m_vtable);
}

void
CategoryRef::setMembersReachable()
{
    for (auto &cell : m_fields) {
        cell.setReachable();
    }
}

void
CategoryRef::clearMembersReachable()
{
    for (auto &cell : m_fields) {
        cell.clearReachable();
    }
}

tempo_utils::Status
category_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<CategoryRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}
