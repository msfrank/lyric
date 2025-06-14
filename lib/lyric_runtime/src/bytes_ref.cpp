
#include <absl/strings/substitute.h>

#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::BytesRef::BytesRef(const LiteralCell &literal)
    : m_owned(false),
      m_reachable(false)
{
    TU_ASSERT (literal.type == LiteralCellType::BYTES);
    m_data = literal.literal.bytes.data;
    m_size = literal.literal.bytes.size;
}

lyric_runtime::BytesRef::BytesRef(const tu_uint8 *src, int32_t size)
    : m_owned(true),
      m_reachable(false)
{
    TU_ASSERT (src != nullptr);
    TU_ASSERT (size >= 0);

    auto *dst = new tu_uint8[size];
    memcpy(dst, src, size);

    m_data = dst;
    m_size = size;
}

lyric_runtime::BytesRef::~BytesRef()
{
    TU_LOG_INFO << "free BytesRef" << BytesRef::toString();
    if (m_owned) {
        delete[] m_data;
    }
}

bool
lyric_runtime::BytesRef::equals(const AbstractRef *other) const
{
    auto *other_ = static_cast<const BytesRef *>(other);

    if (m_size != other_->m_size)
        return false;
    return !memcmp(m_data, other_->m_data, m_size);
}

bool
lyric_runtime::BytesRef::rawSize(tu_int32 &size) const
{
    size = m_size;
    return true;
}

tu_int32
lyric_runtime::BytesRef::rawCopy(tu_int32 offset, char *dst, tu_int32 size)
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
lyric_runtime::BytesRef::utf8Value(std::string &utf8) const
{
    return false;
}

bool
lyric_runtime::BytesRef::hashValue(absl::HashState state)
{
    absl::HashState::combine_contiguous(std::move(state), m_data, m_size);
    return true;
}

tempo_utils::StatusCode
lyric_runtime::BytesRef::errorStatusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::BytesRef::toString() const
{
    return absl::Substitute("<$0: BytesRef size=$1>", this, m_size);
}

lyric_runtime::DataCell
lyric_runtime::BytesRef::byteAt(int index) const
{
    if (m_data == nullptr)
        return lyric_runtime::DataCell::undef();
    if (m_size <= index)
        return lyric_runtime::DataCell::undef();
    auto byte = m_data[index];
    return lyric_runtime::DataCell(static_cast<tu_int64>(byte));
}

lyric_runtime::DataCell
lyric_runtime::BytesRef::bytesCompare(BytesRef *other) const
{
    TU_ASSERT (other != nullptr);

    if (m_data == nullptr && other->m_data == nullptr)
        return lyric_runtime::DataCell(static_cast<int64_t>(0));
    if (m_data == nullptr && other->m_data != nullptr)
        return lyric_runtime::DataCell(static_cast<int64_t>(-1));
    if (m_data != nullptr && other->m_data == nullptr)
        return lyric_runtime::DataCell(static_cast<int64_t>(1));

    auto cmp = std::memcmp(m_data, other->m_data, m_size <= other->m_size? m_size : other->m_size);
    if (cmp != 0)
        return lyric_runtime::DataCell(static_cast<int64_t>(cmp));

    if (m_size < other->m_size)
        return lyric_runtime::DataCell(static_cast<int64_t>(-1));
    if (m_size > other->m_size)
        return lyric_runtime::DataCell(static_cast<int64_t>(1));

    return lyric_runtime::DataCell(static_cast<int64_t>(0));
}

lyric_runtime::DataCell
lyric_runtime::BytesRef::bytesLength() const
{
    return lyric_runtime::DataCell(static_cast<tu_int64>(m_size));
}

const tu_uint8 *
lyric_runtime::BytesRef::getBytesData() const
{
    return m_data;
}

int32_t
lyric_runtime::BytesRef::getBytesSize() const
{
    return m_size;
}

bool
lyric_runtime::BytesRef::isReachable() const
{
    return m_reachable;
}

void
lyric_runtime::BytesRef::setReachable()
{
    m_reachable = true;
}

void
lyric_runtime::BytesRef::clearReachable()
{
    m_reachable = false;
}

const lyric_runtime::VirtualTable *
lyric_runtime::BytesRef::getVirtualTable() const
{
    return nullptr;
}

lyric_runtime::DataCell
lyric_runtime::BytesRef::getField(const DataCell &field) const
{
    return {};
}

lyric_runtime::DataCell
lyric_runtime::BytesRef::setField(const DataCell &field, const DataCell &value)
{
    return {};
}

bool
lyric_runtime::BytesRef::uriValue(tempo_utils::Url &url) const
{
    return false;
}

bool
lyric_runtime::BytesRef::iteratorValid()
{
    return false;
}

bool
lyric_runtime::BytesRef::iteratorNext(DataCell &next)
{
    return false;
}

bool
lyric_runtime::BytesRef::prepareFuture(std::shared_ptr<Promise> promise)
{
    return false;
}

bool
lyric_runtime::BytesRef::awaitFuture(SystemScheduler *systemScheduler)
{
    return false;
}

bool
lyric_runtime::BytesRef::resolveFuture(DataCell &result)
{
    return false;
}

bool
lyric_runtime::BytesRef::applyClosure(Task *task, std::vector<DataCell> &args, InterpreterState *state)
{
    return false;
}

void
lyric_runtime::BytesRef::finalize()
{
}
