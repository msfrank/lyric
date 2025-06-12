
#include <lyric_parser/internal/base_ops.h>

lyric_parser::internal::BaseOps::BaseOps(ModuleArchetype *listener)
    : m_listener(listener)
{
    TU_ASSERT (m_listener != nullptr);
}

lyric_parser::internal::ModuleArchetype *
lyric_parser::internal::BaseOps::getListener() const
{
    return m_listener;
}

lyric_parser::ArchetypeState *
lyric_parser::internal::BaseOps::getState() const
{
    return m_listener->getState();
}

bool
lyric_parser::internal::BaseOps::hasError() const
{
    return m_listener->hasError();
}

void
lyric_parser::internal::BaseOps::logErrorOrThrow(
    size_t lineNr,
    size_t columnNr,
    const std::string &message)
{
    m_listener->logErrorOrThrow(lineNr, columnNr, message);
}

void
lyric_parser::internal::BaseOps::logErrorOrThrow(const antlr4::Token *token, const std::string &message)
{
    auto lineNr = token->getLine();
    auto columnNr = token->getCharPositionInLine();
    m_listener->logErrorOrThrow(lineNr, columnNr, message);
}