#ifndef LYRIC_REWRITER_MACRO_BLOCK_H
#define LYRIC_REWRITER_MACRO_BLOCK_H

#include <lyric_parser/archetype_node.h>

namespace lyric_rewriter {

    class MacroBlock {
    public:
        MacroBlock(lyric_parser::ArchetypeNode *block, int index, lyric_parser::ArchetypeState *state);

        tempo_utils::Status appendNode(lyric_parser::ArchetypeNode *node);

    private:
        lyric_parser::ArchetypeNode *m_block;
        int m_index;
        lyric_parser::ArchetypeState *m_state;
    };
}

#endif // LYRIC_REWRITER_MACRO_BLOCK_H
