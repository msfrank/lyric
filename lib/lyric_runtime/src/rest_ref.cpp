
#include <absl/strings/substitute.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/rest_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::RestRef::RestRef(const ExistentialTable *etable, std::vector<Operand> &&restArgs)
    : m_etable(etable),
      m_restArgs(std::move(restArgs)),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
}

lyric_runtime::RestRef::~RestRef()
{
    TU_LOG_VV << "free RestRef" << RestRef::toString();
}

const lyric_runtime::DescriptorEntry *
lyric_runtime::RestRef::getDescriptorEntry() const
{
    return m_etable->getDescriptorEntry();
}

const lyric_runtime::AbstractMemberResolver *
lyric_runtime::RestRef::getMemberResolver() const
{
    return nullptr;
}

const lyric_runtime::AbstractMethodResolver *
lyric_runtime::RestRef::getMethodResolver() const
{
    return m_etable;
}

const lyric_runtime::AbstractExtensionResolver *
lyric_runtime::RestRef::getExtensionResolver() const
{
    return m_etable;
}

lyric_common::SymbolUrl
lyric_runtime::RestRef::getSymbolUrl() const
{
    return m_etable->getSymbolUrl();
}

size_t
lyric_runtime::RestRef::numRest() const
{
    return m_restArgs.size();
}

lyric_runtime::Operand
lyric_runtime::RestRef::restAt(int index) const
{
    if (0 <= index && index < m_restArgs.size())
        return m_restArgs.at(index);
    return Operand::undef();
}

lyric_runtime::Operand
lyric_runtime::RestRef::restLength() const
{
    return Operand::fromI64(m_restArgs.size());
}

bool
lyric_runtime::RestRef::equals(const AbstractRef *other) const
{
    auto *otherrest = static_cast<const RestRef *>(other);
    if (m_restArgs.size() != otherrest->m_restArgs.size())
        return false;
    for (int i = 0; i < m_restArgs.size(); ++i) {
        const auto &l = m_restArgs.at(i);
        const auto &r = otherrest->m_restArgs.at(i);
        if (!l.isEqualTo(r))
            return false;
    }
    return true;
}

std::string
lyric_runtime::RestRef::toString() const
{
    return absl::Substitute("<$0: RestRef>", this);
}

bool
lyric_runtime::RestRef::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::RestRef::setReachable()
{
    m_reachable = true;
}

void
lyric_runtime::RestRef::clearReachable()
{
    m_reachable = false;
}

void
lyric_runtime::RestRef::finalize()
{
}

bool
lyric_runtime::RestRef::rawSize(tu_int32 &size) const
{
    return false;
}

tu_int32
lyric_runtime::RestRef::rawCopy(tu_int32 offset, char *dst, tu_int32 size) const
{
    return -1;
}

bool
lyric_runtime::RestRef::utf8Value(std::string &utf8) const
{
    return false;
}

bool
lyric_runtime::RestRef::hashValue(absl::HashState state)
{
    for (auto &arg : m_restArgs) {
        state = absl::HashState::combine(std::move(state), OperandEquality(arg));
    }
    return true;
}

tempo_utils::StatusCode
lyric_runtime::RestRef::statusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::RestRef::statusMessage()
{
    return {};
}

bool
lyric_runtime::RestRef::getField(const Operand &field, Operand &value) const
{
    return false;
}

bool
lyric_runtime::RestRef::setField(const Operand &field, const Operand &value, Operand *prev)
{
    return false;
}

bool
lyric_runtime::RestRef::iteratorValid()
{
    return false;
}

bool
lyric_runtime::RestRef::iteratorNext(Operand &next)
{
    return false;
}

bool
lyric_runtime::RestRef::prepareFuture(std::shared_ptr<Promise> promise)
{
    return false;
}

bool
lyric_runtime::RestRef::awaitFuture(SystemScheduler *systemScheduler)
{
    return false;
}

bool
lyric_runtime::RestRef::resolveFuture(Operand &result)
{
    return false;
}

bool
lyric_runtime::RestRef::applyClosure(Task *task, std::vector<Operand> &args, InterpreterState *state)
{
    return false;
}
