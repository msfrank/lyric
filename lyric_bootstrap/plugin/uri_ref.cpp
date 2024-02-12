
#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

#include "uri_ref.h"

UrlRef::UrlRef(const lyric_runtime::VirtualTable *vtable) : BaseRef(vtable)
{
}

UrlRef::~UrlRef()
{
    TU_LOG_INFO << "free UrlRef" << UrlRef::toString();
}

lyric_runtime::DataCell
UrlRef::getField(const lyric_runtime::DataCell &field) const
{
    return lyric_runtime::DataCell();
}

lyric_runtime::DataCell
UrlRef::setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value)
{
    return lyric_runtime::DataCell();
}

bool
UrlRef::equals(const AbstractRef *other) const
{
    return m_uri == static_cast<const UrlRef *>(other)->m_uri;
}

bool
UrlRef::uriValue(tempo_utils::Url &url) const
{
    url = m_uri;
    return true;
}

bool
UrlRef::hashValue(absl::HashState state)
{
    absl::HashState::combine(std::move(state), m_uri);
    return true;
}

bool
UrlRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index)
{
    auto appendValueResult = state.appendValue(tempo_utils::AttrValue(m_uri.toString()));
    if (appendValueResult.isStatus()) {
        index = lyric_runtime::INVALID_ADDRESS_U32;
        return false;
    }
    auto *value = appendValueResult.getResult();
    index = value->getAddress().getAddress();
    return true;
}

std::string
UrlRef::toString() const
{
    return absl::Substitute("<$0: UrlRef \"$1\">", this, m_uri.toString());
}

lyric_runtime::DataCell
UrlRef::uriEquals(UrlRef *other) const
{
    TU_ASSERT (other != nullptr);

    if (!m_uri.isValid() && !other->m_uri.isValid())
        return lyric_runtime::DataCell(true);
    if (!m_uri.isValid() && other->m_uri.isValid())
        return lyric_runtime::DataCell(false);
    if (m_uri.isValid() && !other->m_uri.isValid())
        return lyric_runtime::DataCell(false);
    auto equals = m_uri == other->m_uri;
    return lyric_runtime::DataCell(equals);
}

tempo_utils::Url
UrlRef::getUrl() const
{
    return m_uri;
}

void
UrlRef::setUrl(const tempo_utils::Url &uri)
{
    m_uri = uri;
}

void
UrlRef::setMembersReachable()
{
}

void
UrlRef::clearMembersReachable()
{
}

tempo_utils::Status
uri_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<UrlRef>(vtable);
    currentCoro->pushData(ref);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
uri_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver != nullptr);
    auto *instance = static_cast<UrlRef *>(receiver);

    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::UTF8);

    std::string_view utf8(arg0.data.utf8.data, arg0.data.utf8.size);
    auto uri = tempo_utils::Url::fromString(utf8);
    instance->setUrl(uri);

    return lyric_runtime::InterpreterStatus::ok();
}

tempo_utils::Status
uri_equals(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->peekCall();

    TU_ASSERT(frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.type == lyric_runtime::DataCellType::REF);
    const auto &arg1 = frame.getArgument(1);
    TU_ASSERT(arg1.type == lyric_runtime::DataCellType::REF);

    auto *lhs = static_cast<UrlRef *>(arg0.data.ref);
    auto *rhs = static_cast<UrlRef *>(arg1.data.ref);
    currentCoro->pushData(lhs->uriEquals(rhs));
    return lyric_runtime::InterpreterStatus::ok();
}
