
#include <absl/strings/substitute.h>
#include <utf8.h>

#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/string_ref.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/unicode.h>

lyric_runtime::StringRef::StringRef(const ExistentialTable *etable, std::string_view literal)
    : m_etable(etable),
      m_permanent(false),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
    m_rope = tempo_utils::Rope<char>(literal.cbegin(), literal.cend());
    m_size = m_rope.numElements();
}

lyric_runtime::StringRef::StringRef(const ExistentialTable *etable, const char *src, int32_t size)
    : m_etable(etable),
      m_permanent(false),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
    TU_ASSERT (src != nullptr);
    TU_ASSERT (size >= 0);

    m_rope = tempo_utils::Rope<char>(src, src + size);
    m_size = m_rope.numElements();
}

lyric_runtime::StringRef::StringRef(const ExistentialTable *etable, tempo_utils::Rope<char> rope)
    : m_etable(etable),
      m_permanent(false),
      m_reachable(false)
{
    TU_ASSERT (m_etable != nullptr);
    m_rope = std::move(rope);
    m_size = m_rope.numElements();
}

lyric_runtime::StringRef::~StringRef()
{
    TU_LOG_VV << "free StringRef" << StringRef::toString();
}

const lyric_runtime::DescriptorEntry *
lyric_runtime::StringRef::getDescriptorEntry() const
{
    return m_etable->getDescriptor().data.descriptor;
}

const lyric_runtime::AbstractMemberResolver *
lyric_runtime::StringRef::getMemberResolver() const
{
    return nullptr;
}

const lyric_runtime::AbstractMethodResolver *
lyric_runtime::StringRef::getMethodResolver() const
{
    return m_etable;
}

const lyric_runtime::AbstractExtensionResolver *
lyric_runtime::StringRef::getExtensionResolver() const
{
    return m_etable;
}

lyric_common::SymbolUrl
lyric_runtime::StringRef::getSymbolUrl() const
{
    return m_etable->getSymbolUrl();
}

int
lyric_runtime::StringRef::compare(const AbstractRef *other) const
{
    TU_NOTNULL (other);

    auto thischunks = m_rope.iterateChunks();
    auto *otherstring = static_cast<const StringRef *>(other);
    auto otherchunks = otherstring->m_rope.iterateChunks();

    tempo_utils::RopeChunk<char> thischunk, otherchunk;

    auto thisactive = thischunks.getNext(thischunk);
    auto thisit = thischunk.cbegin();
    auto thisend = thischunk.cend();;

    auto otheractive = otherchunks.getNext(otherchunk);
    auto otherit = otherchunk.cbegin();
    auto otherend = otherchunk.cend();

    while (thisactive && otheractive) {

        while (thisit != thisend && otherit != otherend) {
            try {
                utf8::utfchar32_t thisch = utf8::next(thisit, thisend);
                utf8::utfchar32_t otherch = utf8::next(otherit, otherend);
                if (thisch < otherch)
                    return -1;
                if (otherch < thisch)
                    return 1;
            } catch (utf8::exception &ex) {
            }
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
lyric_runtime::StringRef::equals(const AbstractRef *other) const
{
    return compare(other) == 0;
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
    auto subspan = m_rope.subspan(offset, size);
    auto chunks = subspan.iterateChunks();
    tempo_utils::RopeChunk<char> chunk;

    size_t ncopied = 0;
    while (chunks.getNext(chunk)) {
        memcpy(dst, chunk.data(), chunk.size());
        dst += chunk.size();
        ncopied += chunk.size();
    }
    return ncopied;
}

bool
lyric_runtime::StringRef::utf8Value(std::string &utf8) const
{
    std::string str;

    auto chunks = m_rope.iterateChunks();
    tempo_utils::RopeChunk<char> chunk;
    while (chunks.getNext(chunk)) {
        str.append(chunk.cbegin(), chunk.cend());
    }

    utf8 = std::move(str);
    return true;
}

bool
lyric_runtime::StringRef::hashValue(absl::HashState state)
{
    auto chunks = m_rope.iterateChunks();
    tempo_utils::RopeChunk<char> chunk;
    while (chunks.getNext(chunk)) {
        state = absl::HashState::combine_contiguous(std::move(state), chunk.data(), chunk.size());
    }
    return true;
}

tempo_utils::StatusCode
lyric_runtime::StringRef::statusCode()
{
    return tempo_utils::StatusCode::kOk;
}

std::string
lyric_runtime::StringRef::statusMessage()
{
    return {};
}

std::string
lyric_runtime::StringRef::toString() const
{
    std::string s;
    utf8Value(s);
    return absl::Substitute("<$0: StringRef \"$1\">", this, s);
}

lyric_runtime::DataCell
lyric_runtime::StringRef::stringAt(int index) const
{
    if (m_rope.isEmpty())
        return DataCell::undef();

    try {

        int curr = 0;
        auto chunks = m_rope.iterateChunks();
        tempo_utils::RopeChunk<char> chunk;
        while (chunks.getNext(chunk)) {
            auto *it = chunk.data();
            auto *end = it + chunk.size();
            do {
                utf8::utfchar32_t chr = utf8::next(it, end);
                if (curr++ == index)
                    return DataCell(chr);
            } while (it != end);
        }
        return DataCell::undef();

    } catch (utf8::not_enough_room &ex) {
        return DataCell::undef();
    } catch (utf8::invalid_code_point &ex) {
        return DataCell::undef();
    }
}

lyric_runtime::DataCell
lyric_runtime::StringRef::stringCompare(StringRef *other) const
{
    TU_ASSERT (other != nullptr);
    return DataCell(static_cast<tu_int64>(compare(other)));
}

lyric_runtime::DataCell
lyric_runtime::StringRef::stringLength() const
{
    if (m_rope.isEmpty())
        return DataCell(static_cast<int64_t>(0));

    try {

        size_t length = 0;
        auto chunks = m_rope.iterateChunks();
        tempo_utils::RopeChunk<char> chunk;
        while (chunks.getNext(chunk)) {
            auto *it = chunk.data();
            auto *end = it + chunk.size();
            do {
                utf8::next(it, end);
                length++;
            } while (it != end);
        }
        return DataCell(static_cast<tu_int64>(length));

    } catch (utf8::not_enough_room &ex) {
        return DataCell::undef();
    } catch (utf8::invalid_code_point &ex) {
        return DataCell::undef();
    }
}

tempo_utils::Rope<char>
lyric_runtime::StringRef::getStringData() const
{
    return m_rope;
}

int32_t
lyric_runtime::StringRef::getStringSize() const
{
    return m_size;
}

void
lyric_runtime::StringRef::setPermanent()
{
    m_permanent = true;
}

bool
lyric_runtime::StringRef::isReachable() const
{
    return m_permanent || m_reachable;
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

bool
lyric_runtime::StringRef::getField(const DataCell &field, DataCell &value) const
{
    return false;
}

bool
lyric_runtime::StringRef::setField(const DataCell &field, const DataCell &value, DataCell *prev)
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