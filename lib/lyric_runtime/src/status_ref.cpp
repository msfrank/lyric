#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/status_ref.h>
#include <lyric_runtime/string_ref.h>
#include <tempo_utils/log_stream.h>

lyric_runtime::StatusRef::StatusRef(const VirtualTable *vtable)
    : BaseRef(vtable),
      m_statusCode(tempo_utils::StatusCode::kUnknown)
{
    m_fields.resize(vtable->getLayoutTotal());
}

lyric_runtime::StatusRef::StatusRef(
    const VirtualTable *vtable,
    tempo_utils::StatusCode statusCode,
    StringRef *message)
    : BaseRef(vtable),
      m_statusCode(statusCode),
      m_message(message)
{
    TU_ASSERT (m_message != nullptr);
}

lyric_runtime::StatusRef::~StatusRef()
{
    TU_LOG_VV << "free" << StatusRef::toString();
}

lyric_runtime::DataCell
lyric_runtime::StatusRef::getField(const DataCell &field) const
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
lyric_runtime::StatusRef::setField(const DataCell &field, const DataCell &value)
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

tempo_utils::StatusCode
lyric_runtime::StatusRef::errorStatusCode()
{
    return m_statusCode;
}

lyric_runtime::DataCell
lyric_runtime::StatusRef::getStatusCode() const
{
    return DataCell(static_cast<tu_int64>(m_statusCode));
}

void
lyric_runtime::StatusRef::setStatusCode(tempo_utils::StatusCode statusCode)
{
    switch (statusCode) {
        case tempo_utils::StatusCode::kCancelled:
        case tempo_utils::StatusCode::kInvalidArgument:
        case tempo_utils::StatusCode::kDeadlineExceeded:
        case tempo_utils::StatusCode::kNotFound:
        case tempo_utils::StatusCode::kAlreadyExists:
        case tempo_utils::StatusCode::kPermissionDenied:
        case tempo_utils::StatusCode::kUnauthenticated:
        case tempo_utils::StatusCode::kResourceExhausted:
        case tempo_utils::StatusCode::kFailedPrecondition:
        case tempo_utils::StatusCode::kAborted:
        case tempo_utils::StatusCode::kUnavailable:
        case tempo_utils::StatusCode::kOutOfRange:
        case tempo_utils::StatusCode::kUnimplemented:
        case tempo_utils::StatusCode::kInternal:
        case tempo_utils::StatusCode::kUnknown:
            m_statusCode = statusCode;
            break;
        default:
            m_statusCode = tempo_utils::StatusCode::kUnknown;
            break;
    }
}

std::string
lyric_runtime::StatusRef::errorMessage()
{
    std::string message;
    m_message->utf8Value(message);
    return message;
}

lyric_runtime::DataCell
lyric_runtime::StatusRef::getMessage() const
{
    return DataCell::forString(m_message);
}

void
lyric_runtime::StatusRef::setMessage(const DataCell &message)
{
    TU_ASSERT (message.type == DataCellType::STRING);
    m_message = message.data.str;
}

std::string
lyric_runtime::StatusRef::toString() const
{
    std::string_view message;
    if (m_message != nullptr) {
        message = std::string_view(m_message->getStringData(), m_message->getStringSize());
    }
    return absl::Substitute("<$0: StatusRef $1 message=\"$2\">",
        this,
        BaseRef::getVirtualTable()->getSymbolUrl().toString(),
        message);
}

void
lyric_runtime::StatusRef::setMembersReachable()
{
    if (m_message != nullptr) {
        m_message->setReachable();
    }
    for (auto &cell : m_fields) {
        if (cell.type == DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->setReachable();
        }
    }
}

void
lyric_runtime::StatusRef::clearMembersReachable()
{
    if (m_message != nullptr) {
        m_message->clearReachable();
    }
    for (auto &cell : m_fields) {
        if (cell.type == DataCellType::REF) {
            TU_ASSERT (cell.data.ref != nullptr);
            cell.data.ref->clearReachable();
        }
    }
}