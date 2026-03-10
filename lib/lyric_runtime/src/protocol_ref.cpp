
#include <absl/strings/substitute.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/protocol_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::ProtocolRef::ProtocolRef(const ExistentialTable *etable, const DataCell &descriptor)
    : m_etable(etable),
      m_descriptor(descriptor),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
    TU_ASSERT (m_descriptor.type == DataCellType::DESCRIPTOR);
    TU_ASSERT (m_descriptor.data.descriptor->getLinkageSection() == lyric_object::LinkageSection::Protocol);
}

lyric_runtime::ProtocolRef::~ProtocolRef()
{
    TU_LOG_VV << "free ProtocolRef" << ProtocolRef::toString();
}

const lyric_runtime::AbstractMemberResolver *
lyric_runtime::ProtocolRef::getMemberResolver() const
{
    return nullptr;
}

const lyric_runtime::AbstractMethodResolver *
lyric_runtime::ProtocolRef::getMethodResolver() const
{
    return m_etable;
}

const lyric_runtime::AbstractExtensionResolver *
lyric_runtime::ProtocolRef::getExtensionResolver() const
{
    return m_etable;
}

lyric_common::SymbolUrl
lyric_runtime::ProtocolRef::getSymbolUrl() const
{
    return m_etable->getSymbolUrl();
}

bool
lyric_runtime::ProtocolRef::equals(const AbstractRef *other) const
{
    auto *other_ = static_cast<const ProtocolRef *>(other);
    return m_descriptor == other_->m_descriptor;
}

std::string
lyric_runtime::ProtocolRef::toString() const
{
    return absl::Substitute("<$0: ProtocolRef>", this);
}

bool
lyric_runtime::ProtocolRef::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::ProtocolRef::setReachable()
{
    m_reachable = true;
}

void
lyric_runtime::ProtocolRef::clearReachable()
{
    m_reachable = false;
}

void
lyric_runtime::ProtocolRef::finalize()
{
}

bool
lyric_runtime::ProtocolRef::rawSize(tu_int32 &size) const
{
    return false;
}

tu_int32
lyric_runtime::ProtocolRef::rawCopy(tu_int32 offset, char *dst, tu_int32 size)
{
    return -1;
}

bool
lyric_runtime::ProtocolRef::utf8Value(std::string &utf8) const
{
    return false;
}

bool
lyric_runtime::ProtocolRef::hashValue(absl::HashState state)
{
    absl::HashState::combine(std::move(state), m_descriptor);
    return true;
}

tempo_utils::StatusCode
lyric_runtime::ProtocolRef::errorStatusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::ProtocolRef::errorMessage()
{
    return {};
}

bool
lyric_runtime::ProtocolRef::getField(const DataCell &field, DataCell &value) const
{
    return false;
}

bool
lyric_runtime::ProtocolRef::setField(const DataCell &field, const DataCell &value, DataCell *prev)
{
    return false;
}

bool
lyric_runtime::ProtocolRef::uriValue(tempo_utils::Url &url) const
{
    return false;
}

bool
lyric_runtime::ProtocolRef::iteratorValid()
{
    return false;
}

bool
lyric_runtime::ProtocolRef::iteratorNext(DataCell &next)
{
    return false;
}

bool
lyric_runtime::ProtocolRef::prepareFuture(std::shared_ptr<Promise> promise)
{
    return false;
}

bool
lyric_runtime::ProtocolRef::awaitFuture(SystemScheduler *systemScheduler)
{
    return false;
}

bool
lyric_runtime::ProtocolRef::resolveFuture(DataCell &result)
{
    return false;
}

bool
lyric_runtime::ProtocolRef::applyClosure(Task *task, std::vector<DataCell> &args, InterpreterState *state)
{
    return false;
}
