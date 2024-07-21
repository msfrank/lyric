
#include <lyric_rewriter/macro_block.h>

lyric_rewriter::MacroBlock::MacroBlock(
    lyric_parser::ArchetypeNode *block,
    int index,
    lyric_parser::ArchetypeState *state)
    : m_block(block),
      m_index(index),
      m_state(state)
{
    TU_ASSERT (m_block != nullptr);
    TU_ASSERT (0 <= m_index && m_index < m_block->numChildren());
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Status
lyric_rewriter::MacroBlock::appendNode(lyric_parser::ArchetypeNode *node)
{
    m_block->insertChild(m_index, node);
    m_index++;
    return {};
}
