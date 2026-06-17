
#include <absl/strings/substitute.h>

#include <lyric_runtime/u64_ref.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

lyric_runtime::U64Ref::U64Ref(const ExistentialTable *etable, tu_uint64 u64)
    : m_etable(etable),
      m_u64(u64),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
}

lyric_runtime::U64Ref::~U64Ref()
{
    TU_LOG_VV << "free U64Ref" << U64Ref::toString();
}

const lyric_runtime::DescriptorEntry *
lyric_runtime::U64Ref::getDescriptorEntry() const
{
    return m_etable->getDescriptorEntry();
}

const lyric_runtime::AbstractMemberResolver *
lyric_runtime::U64Ref::getMemberResolver() const
{
    return nullptr;
}

const lyric_runtime::AbstractMethodResolver *
lyric_runtime::U64Ref::getMethodResolver() const
{
    return m_etable;
}

const lyric_runtime::AbstractExtensionResolver *
lyric_runtime::U64Ref::getExtensionResolver() const
{
    return m_etable;
}

lyric_common::SymbolUrl
lyric_runtime::U64Ref::getSymbolUrl() const
{
    return m_etable->getSymbolUrl();
}

int
lyric_runtime::U64Ref::compare(const AbstractRef *other) const
{
    return 0;
}

bool
lyric_runtime::U64Ref::equals(const AbstractRef *other) const
{
    return false;
}

bool
lyric_runtime::U64Ref::rawSize(tu_int32 &size) const
{
    return false;
}

tu_int32
lyric_runtime::U64Ref::rawCopy(tu_int32 offset, char *dst, tu_int32 size) const
{
    return -1;
}

bool
lyric_runtime::U64Ref::utf8Value(std::string &utf8) const
{
    return false;
}

bool
lyric_runtime::U64Ref::hashValue(absl::HashState state)
{
    absl::HashState::combine(std::move(state), m_u64);
    return true;
}

tempo_utils::StatusCode
lyric_runtime::U64Ref::statusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::U64Ref::statusMessage()
{
    return {};
}

std::string
lyric_runtime::U64Ref::toString() const
{
    return absl::Substitute("<$0: U64Ref $1>", this, m_u64);
}

tu_uint64
lyric_runtime::U64Ref::getU64() const
{
    return m_u64;
}

bool
lyric_runtime::U64Ref::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::U64Ref::setReachable()
{
    m_reachable = true;
}

void
lyric_runtime::U64Ref::clearReachable()
{
    m_reachable = false;
}

bool
lyric_runtime::U64Ref::getField(const Operand &field, Operand &value) const
{
    return false;
}

bool
lyric_runtime::U64Ref::setField(const Operand &field, const Operand &value, Operand *prev)
{
    return false;
}

bool
lyric_runtime::U64Ref::iteratorValid()
{
    return false;
}

bool
lyric_runtime::U64Ref::iteratorNext(Operand &next)
{
    return false;
}

bool
lyric_runtime::U64Ref::prepareFuture(std::shared_ptr<Promise> promise)
{
    return false;
}

bool
lyric_runtime::U64Ref::awaitFuture(SystemScheduler *systemScheduler)
{
    return false;
}

bool
lyric_runtime::U64Ref::resolveFuture(Operand &result)
{
    return false;
}

bool
lyric_runtime::U64Ref::applyClosure(Task *task, std::vector<Operand> &args, InterpreterState *state)
{
    return false;
}

void
lyric_runtime::U64Ref::finalize()
{
}
