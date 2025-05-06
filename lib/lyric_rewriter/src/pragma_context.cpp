
#include <lyric_rewriter/pragma_context.h>

lyric_rewriter::PragmaContext::PragmaContext()
{
}

tempo_utils::Status
lyric_rewriter::PragmaContext::appendNode(lyric_parser::ArchetypeNode *node)
{
    TU_ASSERT (node != nullptr);
    m_pragmas.push_back(node);
    return {};
}

std::vector<lyric_parser::ArchetypeNode *>::const_iterator
lyric_rewriter::PragmaContext::pragmasBegin() const
{
    return m_pragmas.cbegin();
}

std::vector<lyric_parser::ArchetypeNode *>::const_iterator
lyric_rewriter::PragmaContext::pragmasEnd() const
{
    return m_pragmas.cend();
}

int
lyric_rewriter::PragmaContext::numPragmas() const
{
    return m_pragmas.size();
}