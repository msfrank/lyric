
#include <absl/strings/substitute.h>

#include <lyric_runtime/f64_ref.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

lyric_runtime::F64Ref::F64Ref(const ExistentialTable *etable, double f64)
    : m_etable(etable),
      m_f64(f64),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
}

lyric_runtime::F64Ref::~F64Ref()
{
    TU_LOG_VV << "free F64Ref" << F64Ref::toString();
}

const lyric_runtime::DescriptorEntry *
lyric_runtime::F64Ref::getDescriptorEntry() const
{
    return m_etable->getDescriptorEntry();
}

const lyric_runtime::AbstractMemberResolver *
lyric_runtime::F64Ref::getMemberResolver() const
{
    return nullptr;
}

const lyric_runtime::AbstractMethodResolver *
lyric_runtime::F64Ref::getMethodResolver() const
{
    return m_etable;
}

const lyric_runtime::AbstractExtensionResolver *
lyric_runtime::F64Ref::getExtensionResolver() const
{
    return m_etable;
}

lyric_common::SymbolUrl
lyric_runtime::F64Ref::getSymbolUrl() const
{
    return m_etable->getSymbolUrl();
}

int
lyric_runtime::F64Ref::compare(const AbstractRef *other) const
{
    return 0;
}

bool
lyric_runtime::F64Ref::equals(const AbstractRef *other) const
{
    return false;
}

bool
lyric_runtime::F64Ref::rawSize(tu_int32 &size) const
{
    return false;
}

tu_int32
lyric_runtime::F64Ref::rawCopy(tu_int32 offset, char *dst, tu_int32 size) const
{
    return -1;
}

bool
lyric_runtime::F64Ref::utf8Value(std::string &utf8) const
{
    return false;
}

bool
lyric_runtime::F64Ref::hashValue(absl::HashState state)
{
    absl::HashState::combine(std::move(state), m_f64);
    return true;
}

tempo_utils::StatusCode
lyric_runtime::F64Ref::statusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::F64Ref::statusMessage()
{
    return {};
}

std::string
lyric_runtime::F64Ref::toString() const
{
    return absl::Substitute("<$0: F64Ref $1>", this, m_f64);
}

double
lyric_runtime::F64Ref::getF64() const
{
    return m_f64;
}

bool
lyric_runtime::F64Ref::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::F64Ref::setReachable()
{
    m_reachable = true;
}

void
lyric_runtime::F64Ref::clearReachable()
{
    m_reachable = false;
}

bool
lyric_runtime::F64Ref::getField(const Operand &field, Operand &value) const
{
    return false;
}

bool
lyric_runtime::F64Ref::setField(const Operand &field, const Operand &value, Operand *prev)
{
    return false;
}

bool
lyric_runtime::F64Ref::iteratorValid()
{
    return false;
}

bool
lyric_runtime::F64Ref::iteratorNext(Operand &next)
{
    return false;
}

bool
lyric_runtime::F64Ref::prepareFuture(std::shared_ptr<Promise> promise)
{
    return false;
}

bool
lyric_runtime::F64Ref::awaitFuture(SystemScheduler *systemScheduler)
{
    return false;
}

bool
lyric_runtime::F64Ref::resolveFuture(Operand &result)
{
    return false;
}

bool
lyric_runtime::F64Ref::applyClosure(Task *task, std::vector<Operand> &args, InterpreterState *state)
{
    return false;
}

void
lyric_runtime::F64Ref::finalize()
{
}
