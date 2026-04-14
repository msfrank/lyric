
#include <lyric_assembler/assembler_result.h>
#include <lyric_assembler/literal_cache.h>

lyric_assembler::LiteralCache::LiteralCache()
{
}

lyric_assembler::LiteralCache::~LiteralCache()
{
    for (auto *literal : m_literals) {
        delete literal;
    }
}

tempo_utils::Result<lyric_assembler::LiteralHandle *>
lyric_assembler::LiteralCache::getOrMakeLiteral(std::string_view utf8)
{
    auto entry = m_stringcache.find(utf8);
    if (entry != m_stringcache.cend()) {
        return m_literals.at(entry->second);
    }

    auto offset = m_literals.size();
    if (offset == std::numeric_limits<int>::max())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "overflowed max literals");

    std::string value(utf8);
    auto *literalPtr = new LiteralHandle(std::move(value));
    m_literals.push_back(literalPtr);
    m_stringcache[literalPtr->literalValue()] = offset;
    return literalPtr;
}

tempo_utils::Result<lyric_assembler::LiteralHandle *>
lyric_assembler::LiteralCache::getOrMakeLiteral(std::span<const tu_uint8> bytes)
{
    std::string_view key((const char *) bytes.data(), bytes.size());

    auto entry = m_stringcache.find(key);
    if (entry != m_stringcache.cend()) {
        return m_literals.at(entry->second);
    }

    auto offset = m_literals.size();
    if (offset == std::numeric_limits<int>::max())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "overflowed max literals");

    std::string value(key);
    auto *literalPtr = new LiteralHandle(std::move(value));
    m_literals.push_back(literalPtr);
    m_stringcache[literalPtr->literalValue()] = offset;
    return literalPtr;
}

tempo_utils::Result<lyric_assembler::LiteralHandle *>
lyric_assembler::LiteralCache::getOrMakeLiteral(const tempo_utils::Url &url)
{
    auto key = url.uriView();

    auto entry = m_stringcache.find(key);
    if (entry != m_stringcache.cend()) {
        return m_literals.at(entry->second);
    }

    auto offset = m_literals.size();
    if (offset == std::numeric_limits<int>::max())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "overflowed max literals");

    std::string value(key);
    auto *literalPtr = new LiteralHandle(std::move(value));
    m_literals.push_back(literalPtr);
    m_stringcache[literalPtr->literalValue()] = offset;
    return literalPtr;
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

int
lyric_assembler::LiteralCache::numLiterals() const
{
    return m_literals.size();
}
