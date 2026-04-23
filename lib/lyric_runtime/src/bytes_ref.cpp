
#include <absl/strings/substitute.h>

#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>
#include <utf8/checked.h>

lyric_runtime::BytesRef::BytesRef(const ExistentialTable *etable, std::string_view literal)
    : m_etable(etable),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
    m_rope = tempo_utils::Rope<tu_uint8>(literal.cbegin(), literal.cend());
    m_size = m_rope.numElements();
}

lyric_runtime::BytesRef::BytesRef(const ExistentialTable *etable, const tu_uint8 *src, int32_t size)
    : m_etable(etable),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
    TU_ASSERT (src != nullptr);
    TU_ASSERT (size >= 0);

    m_rope = tempo_utils::Rope<tu_uint8>(src, src + size);
    m_size = m_rope.numElements();
}

lyric_runtime::BytesRef::BytesRef(const ExistentialTable *etable, tempo_utils::Rope<tu_uint8> rope)
    : m_etable(etable),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
    m_rope = std::move(rope);
    m_size = m_rope.numElements();
}

lyric_runtime::BytesRef::~BytesRef()
{
    TU_LOG_VV << "free BytesRef" << BytesRef::toString();
}

const lyric_runtime::DescriptorEntry *
lyric_runtime::BytesRef::getDescriptorEntry() const
{
    return m_etable->getDescriptor().data.descriptor;
}

const lyric_runtime::AbstractMemberResolver *
lyric_runtime::BytesRef::getMemberResolver() const
{
    return nullptr;
}

const lyric_runtime::AbstractMethodResolver *
lyric_runtime::BytesRef::getMethodResolver() const
{
    return m_etable;
}

const lyric_runtime::AbstractExtensionResolver *
lyric_runtime::BytesRef::getExtensionResolver() const
{
    return m_etable;
}

lyric_common::SymbolUrl
lyric_runtime::BytesRef::getSymbolUrl() const
{
    return m_etable->getSymbolUrl();
}

int
lyric_runtime::BytesRef::compare(const AbstractRef *other) const
{
    TU_NOTNULL (other);

    auto thischunks = m_rope.iterateChunks();
    auto *otherbytes = static_cast<const BytesRef *>(other);
    auto otherchunks = otherbytes->m_rope.iterateChunks();

    tempo_utils::RopeChunk<tu_uint8> thischunk, otherchunk;

    auto thisactive = thischunks.getNext(thischunk);
    auto thisit = thischunk.cbegin();
    auto thisend = thischunk.cend();;

    auto otheractive = otherchunks.getNext(otherchunk);
    auto otherit = otherchunk.cbegin();
    auto otherend = otherchunk.cend();

    while (thisactive && otheractive) {

        while (thisit != thisend && otherit != otherend) {
            if (*thisit < *otherit)
                return -1;
            if (*thisit > *otherit)
                return 1;
            ++thisit;
            ++otherit;
        }

        if (thisit == thisend) {
            thisactive = thischunks.getNext(thischunk);
            thisit = thischunk.cbegin();
            thisend = thischunk.cend();
        }

        if (otherit == otherend) {
            otheractive = otherchunks.getNext(otherchunk);
            otherit = otherchunk.cbegin();
            otherend = otherchunk.cend();
        }
    }

    if (!thisactive)
        return otheractive? 1 : 0;
    return -1;
}

bool
lyric_runtime::BytesRef::equals(const AbstractRef *other) const
{
    return compare(other) == 0;
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
    auto subspan = m_rope.subspan(offset, size);
    auto chunks = subspan.iterateChunks();
    tempo_utils::RopeChunk<tu_uint8> chunk;

    size_t ncopied = 0;
    while (chunks.getNext(chunk)) {
        memcpy(dst, chunk.data(), chunk.size());
        dst += chunk.size();
        ncopied += chunk.size();
    }
    return ncopied;
}

bool
lyric_runtime::BytesRef::utf8Value(std::string &utf8) const
{
    auto chunks = m_rope.iterateChunks();
    tempo_utils::RopeChunk<tu_uint8> chunk;

    while (chunks.getNext(chunk)) {
        utf8.append((const char *) chunk.data(), chunk.size());
    }
    utf8::iterator it(utf8.cbegin(), utf8.cbegin(), utf8.cend());
    utf8::iterator end(utf8.cend(), utf8.cbegin(), utf8.cend());
    try {
        while (it != end) {
            utf8::next(it, end);
        }
        return true;
    } catch (utf8::exception &ex) {
        return false;
    }
}

bool
lyric_runtime::BytesRef::hashValue(absl::HashState state)
{
    auto chunks = m_rope.iterateChunks();
    tempo_utils::RopeChunk<tu_uint8> chunk;
    while (chunks.getNext(chunk)) {
        state = absl::HashState::combine_contiguous(std::move(state), chunk.data(), chunk.size());
    }
    return true;
}

tempo_utils::StatusCode
lyric_runtime::BytesRef::statusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::BytesRef::statusMessage()
{
    return {};
}

std::string
lyric_runtime::BytesRef::toString() const
{
    return absl::Substitute("<$0: BytesRef size=$1>", this, m_size);
}

lyric_runtime::DataCell
lyric_runtime::BytesRef::byteAt(int index) const
{
    if (m_rope.isEmpty())
        return DataCell::undef();

    int curr = 0;
    auto chunks = m_rope.iterateChunks();
    tempo_utils::RopeChunk<tu_uint8> chunk;
    while (chunks.getNext(chunk)) {
        if (index < curr + chunk.size())
            return DataCell(static_cast<tu_int64>(chunk.elementAt(index - curr)));
        curr += chunk.size();
    }
    return DataCell::undef();
}

lyric_runtime::DataCell
lyric_runtime::BytesRef::bytesCompare(BytesRef *other) const
{
    TU_ASSERT (other != nullptr);
    return DataCell(static_cast<tu_int64>(compare(other)));
}

lyric_runtime::DataCell
lyric_runtime::BytesRef::bytesLength() const
{
    return DataCell(static_cast<tu_int64>(m_size));
}

tempo_utils::Rope<tu_uint8>
lyric_runtime::BytesRef::getBytesData() const
{
    return m_rope;
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

bool
lyric_runtime::BytesRef::getField(const DataCell &field, DataCell &value) const
{
    return false;
}

bool
lyric_runtime::BytesRef::setField(const DataCell &field, const DataCell &value, DataCell *prev)
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
