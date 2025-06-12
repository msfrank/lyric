
#include <lyric_parser/internal/semantic_exception.h>
#include <tempo_utils/log_message.h>

lyric_parser::internal::SemanticException::SemanticException(
    const antlr4::Token *token,
    std::string_view message) noexcept
    : m_message(message)
{
    TU_ASSERT (token != nullptr);
    m_lineNr = token->getLine();
    m_columnNr = token->getCharPositionInLine();
}

size_t
lyric_parser::internal::SemanticException::getLineNr() const
{
    return m_lineNr;
}

size_t
lyric_parser::internal::SemanticException::getColumnNr() const
{
    return m_columnNr;
}

std::string
lyric_parser::internal::SemanticException::getMessage() const
{
    return m_message;
}

const char *
lyric_parser::internal::SemanticException::what() const noexcept
{
    return m_message.c_str();
}
