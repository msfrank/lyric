
#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::StringRef::StringRef(const LiteralCell &literal)
    : m_owned(false),
      m_reachable(false)
{
    TU_ASSERT (literal.type == LiteralCellType::UTF8);
    m_data = literal.literal.utf8.data;
    m_size = literal.literal.utf8.size;
}

lyric_runtime::StringRef::StringRef(const char *src, int32_t size)
    : m_owned(true),
      m_reachable(false)
{
    TU_ASSERT (src != nullptr);
    TU_ASSERT (size >= 0);

    auto *dst = new char[size];
    memcpy(dst, src, size);

    m_data = dst;
    m_size = size;
}

lyric_runtime::StringRef::~StringRef()
{
    TU_LOG_INFO << "free StringRef" << StringRef::toString();
    if (m_owned) {
        delete[] m_data;
    }
}

bool
lyric_runtime::StringRef::equals(const AbstractRef *other) const
{
    auto *other_ = static_cast<const StringRef *>(other);

    if (m_size != other_->m_size)
        return false;
    return !memcmp(m_data, other_->m_data, m_size);
}

bool
lyric_runtime::StringRef::rawSize(tu_int32 &size) const
{
    size = m_size;
    return true;
}

tu_int32
lyric_runtime::StringRef::rawCopy(tu_int32 offset, char *dst, tu_int32 size)
{
    if (offset < 0)
        return -1;
    if (dst == nullptr)
        return -1;
    if (size <= 0)
        return -1;

    if (m_size <= offset)
        return 0;

    auto ncopied = size < m_size - offset? size : static_cast<tu_int32>(m_size - offset);
    memcpy(dst, m_data, ncopied);
    return ncopied;
}

bool
lyric_runtime::StringRef::utf8Value(std::string &utf8) const
{
    utf8 = std::string(m_data, m_size);
    return true;
}

bool
lyric_runtime::StringRef::hashValue(absl::HashState state)
{
    absl::HashState::combine_contiguous(std::move(state), m_data, m_size);
    return true;
}

bool
lyric_runtime::StringRef::serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index)
{
    std::string s;
    if (m_data) {
        s = std::string(m_data, m_size);
    }
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
lyric_runtime::StringRef::errorStatusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::StringRef::toString() const
{
    std::string s;
    if (m_data) {
        s = std::string(m_data, m_size);
    }
    return absl::Substitute("<$0: StringRef \"$1\">", this, s);
}

lyric_runtime::DataCell
lyric_runtime::StringRef::stringAt(int index) const
{
    if (m_data == nullptr)
        return lyric_runtime::DataCell::nil();
    UChar32 char32;
    U8_GET_UNSAFE((const tu_uint8 *) m_data, index, char32);
    return lyric_runtime::DataCell(char32);
}

lyric_runtime::DataCell
lyric_runtime::StringRef::stringCompare(StringRef *other) const
{
    TU_ASSERT (other != nullptr);

    if (m_data == nullptr && other->m_data == nullptr)
        return lyric_runtime::DataCell(static_cast<int64_t>(0));
    if (m_data == nullptr && other->m_data != nullptr)
        return lyric_runtime::DataCell(static_cast<int64_t>(-1));
    if (m_data != nullptr && other->m_data == nullptr)
        return lyric_runtime::DataCell(static_cast<int64_t>(1));

    UCharIterator lhs, rhs;
    uiter_setUTF8(&lhs, m_data, m_size);
    uiter_setUTF8(&rhs, other->m_data, other->m_size);

    auto cmp = u_strCompareIter(&lhs, &rhs, true);
    return lyric_runtime::DataCell(static_cast<int64_t>(cmp));
}

lyric_runtime::DataCell
lyric_runtime::StringRef::stringLength() const
{
    tu_int32 length = 0;

    if (m_data != nullptr) {
        tu_int32 index = 0;
        while (index < m_size) {
            U8_FWD_1_UNSAFE(m_data, index);
            length++;
        }
    }

    return lyric_runtime::DataCell(static_cast<int64_t>(length));
}

const char *
lyric_runtime::StringRef::getStringData() const
{
    return m_data;
}

int32_t
lyric_runtime::StringRef::getStringSize() const
{
    return m_size;
}

bool
lyric_runtime::StringRef::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::StringRef::setReachable()
{
    m_reachable = true;
}

void
lyric_runtime::StringRef::clearReachable()
{
    m_reachable = false;
}

const lyric_runtime::VirtualTable *
lyric_runtime::StringRef::getVirtualTable() const
{
    return nullptr;
}

lyric_runtime::DataCell
lyric_runtime::StringRef::getField(const DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
lyric_runtime::StringRef::setField(const DataCell &field, const DataCell &value)
{
    return {};
}

bool
lyric_runtime::StringRef::uriValue(tempo_utils::Url &url) const
{
    return false;
}

bool
lyric_runtime::StringRef::iteratorValid()
{
    return false;
}

bool
lyric_runtime::StringRef::iteratorNext(DataCell &next)
{
    return false;
}

bool
lyric_runtime::StringRef::prepareFuture(std::shared_ptr<Promise> promise)
{
    return false;
}

bool
lyric_runtime::StringRef::awaitFuture(SystemScheduler *systemScheduler)
{
    return false;
}

bool
lyric_runtime::StringRef::resolveFuture(DataCell &result)
{
    return false;
}

bool
lyric_runtime::StringRef::applyClosure(Task *task, std::vector<DataCell> &args, InterpreterState *state)
{
    return false;
}

void
lyric_runtime::StringRef::finalize()
{
}