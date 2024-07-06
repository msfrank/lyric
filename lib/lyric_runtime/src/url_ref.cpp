
#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/url_ref.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::UrlRef::UrlRef(const LiteralCell &literal)
    : m_owned(false),
      m_reachable(false)
{
    TU_ASSERT (literal.type == LiteralCellType::UTF8);

    m_owned = false;
    const auto &utf8 = literal.literal.utf8;
    std::string_view sv(utf8.data, utf8.size);
    m_url = tempo_utils::Url::fromString(sv);
}

lyric_runtime::UrlRef::UrlRef(const tempo_utils::Url &url)
    : m_owned(true),
      m_reachable(false)
{
    TU_ASSERT (url.isValid());
    m_owned = true;
    m_url = url;
}

lyric_runtime::UrlRef::~UrlRef()
{
    TU_LOG_INFO << "free UrlRef" << UrlRef::toString();
}

bool
lyric_runtime::UrlRef::equals(const AbstractRef *other) const
{
    auto *other_ = static_cast<const UrlRef *>(other);
    return m_url == other_->m_url;
}

bool
lyric_runtime::UrlRef::rawSize(tu_int32 &size) const
{
    return false;
}

tu_int32
lyric_runtime::UrlRef::rawCopy(tu_int32 offset, char *dst, tu_int32 size)
{
    return 0;
}

bool
lyric_runtime::UrlRef::utf8Value(std::string &utf8) const
{
    utf8 = m_url.toString();
    return true;
}

bool
lyric_runtime::UrlRef::hashValue(absl::HashState state)
{
    absl::HashState::combine(std::move(state), m_url);
    return true;
}

bool
lyric_runtime::UrlRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index)
{
    std::string s = m_url.toString();
    auto appendValueResult = state.appendValue(tempo_utils::AttrValue(s));
    if (appendValueResult.isStatus()) {
        index = lyric_runtime::INVALID_ADDRESS_U32;
        return false;
    }
    auto *value = appendValueResult.getResult();
    index = value->getAddress().getAddress();
    return true;
}

tempo_utils::StatusCode
lyric_runtime::UrlRef::errorStatusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::UrlRef::toString() const
{
    return absl::Substitute("<$0: UrlRef `$1`>", this, m_url.toString());
}

bool
lyric_runtime::UrlRef::uriValue(tempo_utils::Url &url) const
{
    url = m_url;
    return true;
}

lyric_runtime::DataCell
lyric_runtime::UrlRef::uriEquals(UrlRef *other) const
{
    TU_ASSERT (other != nullptr);

    if (!m_url.isValid() && !other->m_url.isValid())
        return lyric_runtime::DataCell(true);
    if (!m_url.isValid() && other->m_url.isValid())
        return lyric_runtime::DataCell(false);
    if (m_url.isValid() && !other->m_url.isValid())
        return lyric_runtime::DataCell(false);
    auto equals = m_url == other->m_url;
    return lyric_runtime::DataCell(equals);
}

tempo_utils::Url
lyric_runtime::UrlRef::getUrl() const
{
    return m_url;
}

bool
lyric_runtime::UrlRef::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::UrlRef::setReachable()
{
    m_reachable = true;
}

void
lyric_runtime::UrlRef::clearReachable()
{
    m_reachable = false;
}

const lyric_runtime::VirtualTable *
lyric_runtime::UrlRef::getVirtualTable() const
{
    return nullptr;
}

lyric_runtime::DataCell
lyric_runtime::UrlRef::getField(const DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
lyric_runtime::UrlRef::setField(const DataCell &field, const DataCell &value)
{
    return {};
}

bool
lyric_runtime::UrlRef::iteratorValid()
{
    return false;
}

bool
lyric_runtime::UrlRef::iteratorNext(DataCell &next)
{
    return false;
}

bool
lyric_runtime::UrlRef::prepareFuture(std::shared_ptr<Promise> promise)
{
    return false;
}

bool
lyric_runtime::UrlRef::awaitFuture(SystemScheduler *systemScheduler)
{
    return false;
}

bool
lyric_runtime::UrlRef::resolveFuture(DataCell &result)
{
    return false;
}

bool
lyric_runtime::UrlRef::applyClosure(Task *task, std::vector<DataCell> &args, InterpreterState *state)
{
    return false;
}

void
lyric_runtime::UrlRef::finalize()
{
}