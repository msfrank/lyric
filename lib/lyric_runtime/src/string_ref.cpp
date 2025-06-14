
#include <absl/strings/substitute.h>
#include <utf8.h>

#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_serde/patchset_value.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

inline utf8::iterator<const char *>
make_start_iterator(const lyric_runtime::StringRef *ref, tu_uint32 start = 0)
{
    TU_ASSERT (ref != nullptr);
    auto *data = ref->getStringData();
    TU_ASSERT (data != nullptr);
    auto size = ref->getStringSize();
    TU_ASSERT (size >= 0);
    auto *rangestart = start < size? data + start : data + size;
    return utf8::iterator(data, rangestart, data + size);
}

inline utf8::iterator<const char *>
make_end_iterator(const lyric_runtime::StringRef *ref)
{
    TU_ASSERT (ref != nullptr);
    auto *data = ref->getStringData();
    TU_ASSERT (data != nullptr);
    auto size = ref->getStringSize();
    TU_ASSERT (size >= 0);
    return utf8::iterator(data + size, data, data + size);
}

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
    auto *otherstring = static_cast<const StringRef *>(other);
    TU_ASSERT (otherstring != nullptr);
    if (m_size != otherstring->m_size)
        return false;
    return !memcmp(m_data, otherstring->m_data, m_size);
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
    auto appendValueResult = state.appendValue(tempo_schema::AttrValue(s));
    if (appendValueResult.isStatus()) {
        index = INVALID_ADDRESS_U32;
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
        return DataCell::undef();
    try {
        auto *it = (char *) m_data;
        auto *end = it + m_size;
        utf8::advance(it, index, end);
        auto chr = utf8::peek_next(it, end);
        return DataCell(chr);
    } catch (utf8::not_enough_room &ex) {
        return DataCell::undef();
    } catch (utf8::invalid_code_point &ex) {
        return DataCell::undef();
    }
}

inline tu_int64
lexographical_compare(const lyric_runtime::StringRef *ref1, const lyric_runtime::StringRef *ref2)
{
    auto it1 = make_start_iterator(ref1);
    auto end1 = make_end_iterator(ref1);
    auto it2 = make_start_iterator(ref2);
    auto end2 = make_end_iterator(ref2);

    for (; (it1 != end1) && (it2 != end2); ++it1, ++it2) {
        if (*it1 < *it2)
            return -1;
        if (*it2 < *it1)
            return 1;
    }

    if (it1 == end1) {
        return it2 != end2? 1 : 0;
    }
    return it2 != end2? 0 : -1;
}

lyric_runtime::DataCell
lyric_runtime::StringRef::stringCompare(StringRef *other) const
{
    TU_ASSERT (other != nullptr);
    return DataCell(lexographical_compare(this, other));
}

lyric_runtime::DataCell
lyric_runtime::StringRef::stringLength() const
{
    if (m_data == nullptr || m_size == 0)
        return DataCell(static_cast<int64_t>(0));

    auto it = make_start_iterator(this);
    auto end = make_end_iterator(this);
    tu_int32 length = 0;
    while (it != end) {
        utf8::next(it, end);
        length++;
    }
    return DataCell(static_cast<int64_t>(length));
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