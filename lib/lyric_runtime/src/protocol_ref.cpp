
#include <absl/strings/substitute.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/protocol_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::ProtocolRef::ProtocolRef(
    const ExistentialTable *etable,
    const lyric_common::SymbolUrl &protocolUrl,
    DescriptorEntry *descriptorEntry,
    TypeEntry *typeEntry,
    lyric_object::PortType port,
    lyric_object::CommunicationType comm)
    : m_etable(etable),
      m_url(protocolUrl),
      m_descriptor(descriptorEntry),
      m_type(typeEntry),
      m_port(port),
      m_comm(comm),
      m_reachable(false)
{
    TU_NOTNULL (m_etable);
    TU_NOTNULL (m_descriptor);
    TU_NOTNULL (m_type);
    TU_ASSERT (m_descriptor->getLinkageSection() == lyric_object::LinkageSection::Protocol);
    TU_ASSERT (m_url.isValid());
    TU_ASSERT (m_port != lyric_object::PortType::Invalid);
    TU_ASSERT (m_comm != lyric_object::CommunicationType::Invalid);
}

lyric_runtime::ProtocolRef::~ProtocolRef()
{
    TU_LOG_VV << "free ProtocolRef" << ProtocolRef::toString();
}

const lyric_runtime::DescriptorEntry *
lyric_runtime::ProtocolRef::getDescriptorEntry() const
{
    return m_etable->getDescriptorEntry();
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

lyric_runtime::Operand
lyric_runtime::ProtocolRef::protocolIsAcceptor() const
{
    return Operand::fromBool(m_port == lyric_object::PortType::Accept);
}

lyric_runtime::Operand
lyric_runtime::ProtocolRef::protocolIsConnector() const
{
    return Operand::fromBool(m_port == lyric_object::PortType::Connect);
}

lyric_runtime::Operand
lyric_runtime::ProtocolRef::protocolCanSend() const
{
    switch (m_comm) {
        case lyric_object::CommunicationType::Send:
        case lyric_object::CommunicationType::SendAndReceive:
            return Operand::fromBool(true);
        default:
            return Operand::fromBool(false);
    }
}

lyric_runtime::Operand
lyric_runtime::ProtocolRef::protocolCanReceive() const
{
    switch (m_comm) {
        case lyric_object::CommunicationType::Receive:
        case lyric_object::CommunicationType::SendAndReceive:
            return Operand::fromBool(true);
        default:
            return Operand::fromBool(false);
    }
}

lyric_runtime::Operand
lyric_runtime::ProtocolRef::protocolType() const
{
    return Operand::fromType(m_type);
}

lyric_common::SymbolUrl
lyric_runtime::ProtocolRef::getSymbolUrl() const
{
    return m_url;
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
lyric_runtime::ProtocolRef::rawCopy(tu_int32 offset, char *dst, tu_int32 size) const
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
lyric_runtime::ProtocolRef::statusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::ProtocolRef::statusMessage()
{
    return {};
}

bool
lyric_runtime::ProtocolRef::getField(const Operand &field, Operand &value) const
{
    return false;
}

bool
lyric_runtime::ProtocolRef::setField(const Operand &field, const Operand &value, Operand *prev)
{
    return false;
}

bool
lyric_runtime::ProtocolRef::iteratorValid()
{
    return false;
}

bool
lyric_runtime::ProtocolRef::iteratorNext(Operand &next)
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
lyric_runtime::ProtocolRef::resolveFuture(Operand &result)
{
    return false;
}

bool
lyric_runtime::ProtocolRef::applyClosure(Task *task, std::vector<Operand> &args, InterpreterState *state)
{
    return false;
}
