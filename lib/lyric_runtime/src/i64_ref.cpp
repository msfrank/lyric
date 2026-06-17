
#include <absl/strings/substitute.h>

#include <lyric_runtime/i64_ref.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

lyric_runtime::I64Ref::I64Ref(const ExistentialTable *etable, tu_int64 i64)
    : m_etable(etable),
      m_i64(i64),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
}

lyric_runtime::I64Ref::~I64Ref()
{
    TU_LOG_VV << "free I64Ref" << I64Ref::toString();
}

const lyric_runtime::DescriptorEntry *
lyric_runtime::I64Ref::getDescriptorEntry() const
{
    return m_etable->getDescriptorEntry();
}

const lyric_runtime::AbstractMemberResolver *
lyric_runtime::I64Ref::getMemberResolver() const
{
    return nullptr;
}

const lyric_runtime::AbstractMethodResolver *
lyric_runtime::I64Ref::getMethodResolver() const
{
    return m_etable;
}

const lyric_runtime::AbstractExtensionResolver *
lyric_runtime::I64Ref::getExtensionResolver() const
{
    return m_etable;
}

lyric_common::SymbolUrl
lyric_runtime::I64Ref::getSymbolUrl() const
{
    return m_etable->getSymbolUrl();
}

int
lyric_runtime::I64Ref::compare(const AbstractRef *other) const
{
    return 0;
}

bool
lyric_runtime::I64Ref::equals(const AbstractRef *other) const
{
    return false;
}

bool
lyric_runtime::I64Ref::rawSize(tu_int32 &size) const
{
    return false;
}

tu_int32
lyric_runtime::I64Ref::rawCopy(tu_int32 offset, char *dst, tu_int32 size) const
{
    return -1;
}

bool
lyric_runtime::I64Ref::utf8Value(std::string &utf8) const
{
    return false;
}

bool
lyric_runtime::I64Ref::hashValue(absl::HashState state)
{
    absl::HashState::combine(std::move(state), m_i64);
    return true;
}

tempo_utils::StatusCode
lyric_runtime::I64Ref::statusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::I64Ref::statusMessage()
{
    return {};
}

std::string
lyric_runtime::I64Ref::toString() const
{
    return absl::Substitute("<$0: I64Ref $1>", this, m_i64);
}

tu_int64
lyric_runtime::I64Ref::getI64() const
{
    return m_i64;
}

bool
lyric_runtime::I64Ref::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::I64Ref::setReachable()
{
    m_reachable = true;
}

void
lyric_runtime::I64Ref::clearReachable()
{
    m_reachable = false;
}

bool
lyric_runtime::I64Ref::getField(const Operand &field, Operand &value) const
{
    return false;
}

bool
lyric_runtime::I64Ref::setField(const Operand &field, const Operand &value, Operand *prev)
{
    return false;
}

bool
lyric_runtime::I64Ref::iteratorValid()
{
    return false;
}

bool
lyric_runtime::I64Ref::iteratorNext(Operand &next)
{
    return false;
}

bool
lyric_runtime::I64Ref::prepareFuture(std::shared_ptr<Promise> promise)
{
    return false;
}

bool
lyric_runtime::I64Ref::awaitFuture(SystemScheduler *systemScheduler)
{
    return false;
}

bool
lyric_runtime::I64Ref::resolveFuture(Operand &result)
{
    return false;
}

bool
lyric_runtime::I64Ref::applyClosure(Task *task, std::vector<Operand> &args, InterpreterState *state)
{
    return false;
}

void
lyric_runtime::I64Ref::finalize()
{
}
