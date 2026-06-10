
#include <absl/strings/substitute.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/namespace_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::NamespaceRef::NamespaceRef(
    const ExistentialTable *etable,
    const lyric_common::SymbolUrl &namespaceUrl,
    DescriptorEntry *descriptorEntry,
    TypeEntry *typeEntry)
    : m_etable(etable),
      m_url(namespaceUrl),
      m_descriptor(descriptorEntry),
      m_type(typeEntry),
      m_reachable(false)
{
    TU_NOTNULL (m_etable);
    TU_NOTNULL (m_descriptor);
    TU_NOTNULL (m_type);
    TU_ASSERT (m_descriptor->getLinkageSection() == lyric_object::LinkageSection::Namespace);
    TU_ASSERT (m_url.isValid());
}

lyric_runtime::NamespaceRef::~NamespaceRef()
{
    TU_LOG_VV << "free NamespaceRef" << NamespaceRef::toString();
}

const lyric_runtime::DescriptorEntry *
lyric_runtime::NamespaceRef::getDescriptorEntry() const
{
    return m_etable->getDescriptorEntry();
}

const lyric_runtime::AbstractMemberResolver *
lyric_runtime::NamespaceRef::getMemberResolver() const
{
    return nullptr;
}

const lyric_runtime::AbstractMethodResolver *
lyric_runtime::NamespaceRef::getMethodResolver() const
{
    return m_etable;
}

const lyric_runtime::AbstractExtensionResolver *
lyric_runtime::NamespaceRef::getExtensionResolver() const
{
    return m_etable;
}

lyric_runtime::Operand
lyric_runtime::NamespaceRef::namespaceType() const
{
    return Operand::fromType(m_type);
}

lyric_common::SymbolUrl
lyric_runtime::NamespaceRef::getSymbolUrl() const
{
    return m_url;
}

bool
lyric_runtime::NamespaceRef::equals(const AbstractRef *other) const
{
    auto *other_ = static_cast<const NamespaceRef *>(other);
    return m_descriptor == other_->m_descriptor;
}

std::string
lyric_runtime::NamespaceRef::toString() const
{
    return absl::Substitute("<$0: NamespaceRef>", this);
}

bool
lyric_runtime::NamespaceRef::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::NamespaceRef::setReachable()
{
    m_reachable = true;
}

void
lyric_runtime::NamespaceRef::clearReachable()
{
    m_reachable = false;
}

void
lyric_runtime::NamespaceRef::finalize()
{
}

bool
lyric_runtime::NamespaceRef::rawSize(tu_int32 &size) const
{
    return false;
}

tu_int32
lyric_runtime::NamespaceRef::rawCopy(tu_int32 offset, char *dst, tu_int32 size) const
{
    return -1;
}

bool
lyric_runtime::NamespaceRef::utf8Value(std::string &utf8) const
{
    return false;
}

bool
lyric_runtime::NamespaceRef::hashValue(absl::HashState state)
{
    absl::HashState::combine(std::move(state), m_descriptor);
    return true;
}

tempo_utils::StatusCode
lyric_runtime::NamespaceRef::statusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::NamespaceRef::statusMessage()
{
    return {};
}

bool
lyric_runtime::NamespaceRef::getField(const Operand &field, Operand &value) const
{
    return false;
}

bool
lyric_runtime::NamespaceRef::setField(const Operand &field, const Operand &value, Operand *prev)
{
    return false;
}

bool
lyric_runtime::NamespaceRef::iteratorValid()
{
    return false;
}

bool
lyric_runtime::NamespaceRef::iteratorNext(Operand &next)
{
    return false;
}

bool
lyric_runtime::NamespaceRef::prepareFuture(std::shared_ptr<Promise> promise)
{
    return false;
}

bool
lyric_runtime::NamespaceRef::awaitFuture(SystemScheduler *systemScheduler)
{
    return false;
}

bool
lyric_runtime::NamespaceRef::resolveFuture(Operand &result)
{
    return false;
}

bool
lyric_runtime::NamespaceRef::applyClosure(Task *task, std::vector<Operand> &args, InterpreterState *state)
{
    return false;
}
