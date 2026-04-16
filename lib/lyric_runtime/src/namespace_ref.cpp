
#include <absl/strings/substitute.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/namespace_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::NamespaceRef::NamespaceRef(
    const ExistentialTable *etable,
    const DataCell &descriptor,
    const DataCell &type)
    : m_etable(etable),
      m_descriptor(descriptor),
      m_type(type),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
    TU_ASSERT (m_descriptor.type == DataCellType::DESCRIPTOR);
    TU_ASSERT (m_descriptor.data.descriptor->getLinkageSection() == lyric_object::LinkageSection::Namespace);
    TU_ASSERT (m_type.type == DataCellType::TYPE);
}

lyric_runtime::NamespaceRef::~NamespaceRef()
{
    TU_LOG_VV << "free NamespaceRef" << NamespaceRef::toString();
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

lyric_runtime::DataCell
lyric_runtime::NamespaceRef::namespaceType() const
{
    return m_type;
}

lyric_common::SymbolUrl
lyric_runtime::NamespaceRef::getSymbolUrl() const
{
    return m_etable->getSymbolUrl();
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
lyric_runtime::NamespaceRef::rawCopy(tu_int32 offset, char *dst, tu_int32 size)
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
lyric_runtime::NamespaceRef::getField(const DataCell &field, DataCell &value) const
{
    return false;
}

bool
lyric_runtime::NamespaceRef::setField(const DataCell &field, const DataCell &value, DataCell *prev)
{
    return false;
}

bool
lyric_runtime::NamespaceRef::iteratorValid()
{
    return false;
}

bool
lyric_runtime::NamespaceRef::iteratorNext(DataCell &next)
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
lyric_runtime::NamespaceRef::resolveFuture(DataCell &result)
{
    return false;
}

bool
lyric_runtime::NamespaceRef::applyClosure(Task *task, std::vector<DataCell> &args, InterpreterState *state)
{
    return false;
}
