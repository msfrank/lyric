
#include <lyric_assembler/literal_cache.h>

lyric_assembler::LiteralCache::LiteralCache(AssemblerTracer *tracer)
    : m_tracer(tracer)
{
    TU_ASSERT (m_tracer != nullptr);
}

lyric_assembler::LiteralCache::~LiteralCache()
{
    for (auto &ptr : m_literals) {
        delete ptr;
    }
}

tempo_utils::Result<lyric_assembler::LiteralHandle *>
lyric_assembler::LiteralCache::makeNil()
{
    lyric_runtime::LiteralCell literalCell;
    if (m_literalcache.contains(literalCell)) {
        auto offset = m_literalcache[literalCell];
        return m_literals.at(offset);
    }

    auto offset = m_literals.size();
    if (offset == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    auto *literalHandle = new LiteralHandle();
    m_literals.push_back(literalHandle);

    m_literalcache[literalCell] = offset;
    return literalHandle;
}

tempo_utils::Result<lyric_assembler::LiteralHandle *>
lyric_assembler::LiteralCache::makeBool(bool b)
{
    lyric_runtime::LiteralCell literalCell(b);
    if (m_literalcache.contains(literalCell)) {
        auto offset = m_literalcache[literalCell];
        return m_literals.at(offset);
    }

    auto offset = m_literals.size();
    if (offset == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    auto *literalHandle = new LiteralHandle(b);
    m_literals.push_back(literalHandle);

    m_literalcache[literalCell] = offset;
    return literalHandle;
}

tempo_utils::Result<lyric_assembler::LiteralHandle *>
lyric_assembler::LiteralCache::makeInteger(tu_int64 i64)
{
    lyric_runtime::LiteralCell literalCell(i64);
    if (m_literalcache.contains(literalCell)) {
        auto offset = m_literalcache[literalCell];
        return m_literals.at(offset);
    }

    auto offset = m_literals.size();
    if (offset == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    auto *literalHandle = new LiteralHandle(i64);
    m_literals.push_back(literalHandle);

    m_literalcache[literalCell] = offset;
    return literalHandle;
}

tempo_utils::Result<lyric_assembler::LiteralHandle *>
lyric_assembler::LiteralCache::makeFloat(double dbl)
{
    lyric_runtime::LiteralCell literalCell(dbl);
    if (m_literalcache.contains(literalCell)) {
        auto offset = m_literalcache[literalCell];
        return m_literals.at(offset);
    }

    auto offset = m_literals.size();
    if (offset == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    auto *literalHandle = new LiteralHandle(dbl);
    m_literals.push_back(literalHandle);

    m_literalcache[literalCell] = offset;
    return literalHandle;
}

tempo_utils::Result<lyric_assembler::LiteralHandle *>
lyric_assembler::LiteralCache::makeChar(UChar32 chr)
{
    lyric_runtime::LiteralCell literalCell(chr);
    if (m_literalcache.contains(literalCell)) {
        auto offset = m_literalcache[literalCell];
        return m_literals.at(offset);
    }

    auto offset = m_literals.size();
    if (offset == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    auto *literalHandle = new LiteralHandle(chr);
    m_literals.push_back(literalHandle);

    m_literalcache[literalCell] = offset;
    return literalHandle;
}

tempo_utils::Result<lyric_assembler::LiteralHandle *>
lyric_assembler::LiteralCache::makeUtf8(const std::string &utf8)
{
    if (m_stringcache.contains(utf8)) {
        auto offset = m_stringcache[utf8];
        return m_literals.at(offset);
    }

    auto offset = m_literals.size();
    if (offset == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    auto *literalHandle = new LiteralHandle(std::make_shared<const std::string>(utf8));
    m_literals.push_back(literalHandle);
    m_stringcache[utf8] = offset;
    return literalHandle;
}

std::vector<lyric_assembler::LiteralHandle *>::const_iterator
lyric_assembler::LiteralCache::literalsBegin() const
{
    return m_literals.cbegin();
}

std::vector<lyric_assembler::LiteralHandle *>::const_iterator
lyric_assembler::LiteralCache::literalsEnd() const
{
    return m_literals.cend();
}
