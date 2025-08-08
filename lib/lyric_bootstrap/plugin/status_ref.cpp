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
    TU_LOG_VV << "free" << StatusRef::toString();
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

tempo_utils::StatusCode
StatusRef::errorStatusCode()
{
    return m_statusCode;
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
StatusRef::setStatusCode(tempo_utils::StatusCode statusCode)
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

    auto &frame = currentCoro->currentCallOrThrow();
    const auto *vtable = frame.getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto ref = state->heapManager()->allocateRef<StatusRef>(vtable);
    currentCoro->pushData(ref);

    return {};
}

tempo_utils::Status
status_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    TU_ASSERT(receiver.type == lyric_runtime::DataCellType::REF);
    auto *instance = static_cast<StatusRef *>(receiver.data.ref);

    TU_ASSERT (frame.numArguments() > 0);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT (arg0.type == lyric_runtime::DataCellType::I64);
    auto statusCode = static_cast<tempo_utils::StatusCode>(arg0.data.i64);

    instance->setStatusCode(statusCode);

    return {};
}
