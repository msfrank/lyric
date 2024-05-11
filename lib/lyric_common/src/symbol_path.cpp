
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>

#include <lyric_common/symbol_path.h>
#include <tempo_utils/log_stream.h>

lyric_common::SymbolPath::SymbolPath()
    : m_priv(std::make_shared<Priv>())
{
}

lyric_common::SymbolPath::SymbolPath(const std::vector<std::string> &symbolPath)
    : m_priv(std::make_shared<Priv>(symbolPath.cbegin(), symbolPath.cend()))
{
}
lyric_common::SymbolPath::SymbolPath(std::initializer_list<std::string> &symbolPath)
    : m_priv(std::make_shared<Priv>(symbolPath.begin(), symbolPath.end()))
{
}

lyric_common::SymbolPath::SymbolPath(const std::vector<std::string> &symbolEnclosure, std::string_view symbolName)
{
    std::vector<std::string> path(symbolEnclosure.cbegin(), symbolEnclosure.cend());
    path.emplace_back(symbolName);
    m_priv = std::make_shared<Priv>(std::move(path));
}

lyric_common::SymbolPath::SymbolPath(std::initializer_list<std::string> &symbolEnclosure, std::string_view symbolName)
{
    std::vector<std::string> path(symbolEnclosure.begin(), symbolEnclosure.end());
    path.emplace_back(symbolName);
    m_priv = std::make_shared<Priv>(std::move(path));
}

lyric_common::SymbolPath::SymbolPath(const lyric_common::SymbolPath &other)
    : m_priv(other.m_priv)
{
}

lyric_common::SymbolPath::SymbolPath(SymbolPath &&other) noexcept
{
    m_priv = std::move(other.m_priv);
}

lyric_common::SymbolPath &
lyric_common::SymbolPath::operator=(const SymbolPath &other)
{
    if (this != &other) {
        m_priv = other.m_priv;
    }
    return *this;
}

lyric_common::SymbolPath &
lyric_common::SymbolPath::operator=(SymbolPath &&other) noexcept
{
    if (this != &other) {
        m_priv = std::move(other.m_priv);
    }
    return *this;
}

bool
lyric_common::SymbolPath::isValid() const
{
    return !m_priv->parts->empty();
}

std::vector<std::string>
lyric_common::SymbolPath::getPath() const
{
    return std::vector<std::string>(m_priv->parts->cbegin(), m_priv->parts->cend());
}

std::vector<std::string>
lyric_common::SymbolPath::getEnclosure() const
{
    if (m_priv->parts->empty())
        return {};
    std::vector<std::string> symbolPath(m_priv->parts->cbegin(), --m_priv->parts->cend());
//    std::vector<std::string> symbolPath(m_priv->parts->cbegin(), m_priv->parts->cend());
//    symbolPath.pop_back();
    return symbolPath;
}

std::string
lyric_common::SymbolPath::getName() const
{
    if (m_priv->parts->empty())
        return {};
    return m_priv->parts->back();
}

std::string
lyric_common::SymbolPath::toString() const
{
    return absl::StrJoin(*m_priv->parts, ".");
}

bool
lyric_common::SymbolPath::operator==(const lyric_common::SymbolPath &other) const
{
    return *m_priv->parts == *other.m_priv->parts;
}

bool
lyric_common::SymbolPath::operator!=(const lyric_common::SymbolPath &other) const
{
    return !(*this == other);
}

lyric_common::SymbolPath
lyric_common::SymbolPath::fromString(std::string_view s)
{
    std::vector<std::string> parts;
    for (const auto &part : absl::StrSplit(s, ".", absl::SkipEmpty())) {
        parts.emplace_back(part.data(), part.size());
    }
    return lyric_common::SymbolPath(parts);
}

lyric_common::SymbolPath
lyric_common::SymbolPath::entrySymbol()
{
    return lyric_common::SymbolPath({}, "$entry");
}

tempo_utils::LogMessage&&
lyric_common::operator<<(tempo_utils::LogMessage &&message, const lyric_common::SymbolPath &symbolPath)
{
    std::forward<tempo_utils::LogMessage>(message) << symbolPath.toString();
    return std::move(message);
}