#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

#include "singleton_ref.h"

SingletonRef::SingletonRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
    m_fields.resize(vtable->getLayoutTotal());
}

SingletonRef::~SingletonRef()
{
    TU_LOG_VV << "free" << SingletonRef::toString();
}

bool
SingletonRef::getField(const lyric_runtime::Operand &field, lyric_runtime::Operand &value) const
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
SingletonRef::setField(
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
SingletonRef::toString() const
{
    return absl::Substitute("<$0: SingletonRef $1 vtable=$2>",
        this,
        lyric_runtime::BaseRef::getVirtualTable()->getSymbolUrl().toString(),
        m_vtable);
}

void
SingletonRef::setMembersReachable()
{
    for (auto &cell : m_fields) {
        cell.setReachable();
    }
}

void
SingletonRef::clearMembersReachable()
{
    for (auto &cell : m_fields) {
        cell.clearReachable();
    }
}

tempo_utils::Status
singleton_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<SingletonRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}
