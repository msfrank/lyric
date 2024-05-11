
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

tempo_utils::Result<lyric_assembler::LiteralAddress>
lyric_assembler::LiteralCache::makeLiteralNil()
{
    lyric_runtime::LiteralCell literalCell;
    if (m_literalcache.contains(literalCell))
        return m_literalcache[literalCell];

    auto length = m_literals.size();
    if (length == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    LiteralAddress literalAddress(static_cast<uint32_t>(length));
    auto *literalHandle = new LiteralHandle(literalAddress);
    m_literals.push_back(literalHandle);

    m_literalcache[literalCell] = literalAddress;
    return literalAddress;
}

tempo_utils::Result<lyric_assembler::LiteralAddress>
lyric_assembler::LiteralCache::makeLiteralBool(bool b)
{
    lyric_runtime::LiteralCell literalCell(b);
    if (m_literalcache.contains(literalCell))
        return m_literalcache[literalCell];

    auto length = m_literals.size();
    if (length == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    LiteralAddress literalAddress(static_cast<uint32_t>(length));
    auto *literalHandle = new LiteralHandle(literalAddress, b);
    m_literals.push_back(literalHandle);

    m_literalcache[literalCell] = literalAddress;
    return literalAddress;
}

tempo_utils::Result<lyric_assembler::LiteralAddress>
lyric_assembler::LiteralCache::makeLiteralInteger(int64_t i64)
{
    lyric_runtime::LiteralCell literalCell(i64);
    if (m_literalcache.contains(literalCell))
        return m_literalcache[literalCell];

    auto length = m_literals.size();
    if (length == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    LiteralAddress literalAddress(static_cast<uint32_t>(length));
    auto *literalHandle = new LiteralHandle(literalAddress, i64);
    m_literals.push_back(literalHandle);

    m_literalcache[literalCell] = literalAddress;
    return literalAddress;
}

tempo_utils::Result<lyric_assembler::LiteralAddress>
lyric_assembler::LiteralCache::makeLiteralFloat(double dbl)
{
    lyric_runtime::LiteralCell literalCell(dbl);
    if (m_literalcache.contains(literalCell))
        return m_literalcache[literalCell];

    auto length = m_literals.size();
    if (length == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    LiteralAddress literalAddress(static_cast<uint32_t>(length));
    auto *literalHandle = new LiteralHandle(literalAddress, dbl);
    m_literals.push_back(literalHandle);

    m_literalcache[literalCell] = literalAddress;
    return literalAddress;
}

tempo_utils::Result<lyric_assembler::LiteralAddress>
lyric_assembler::LiteralCache::makeLiteralChar(UChar32 chr)
{
    lyric_runtime::LiteralCell literalCell(chr);
    if (m_literalcache.contains(literalCell))
        return m_literalcache[literalCell];

    auto length = m_literals.size();
    if (length == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    LiteralAddress literalAddress(static_cast<uint32_t>(length));
    auto *literalHandle = new LiteralHandle(literalAddress, chr);
    m_literals.push_back(literalHandle);

    m_literalcache[literalCell] = literalAddress;
    return literalAddress;
}

tempo_utils::Result<lyric_assembler::LiteralAddress>
lyric_assembler::LiteralCache::makeLiteralUtf8(const std::string &utf8)
{
    if (m_stringcache.contains(utf8))
        return m_stringcache[utf8];

    auto length = m_literals.size();
    if (length == std::numeric_limits<int>::max())
        m_tracer->throwAssemblerInvariant("overflowed max literals");

    LiteralAddress literalAddress(static_cast<uint32_t>(length));
    auto *literalHandle = new LiteralHandle(literalAddress, utf8);
    m_literals.push_back(literalHandle);
    m_stringcache[utf8] = literalAddress;
    return literalAddress;
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
