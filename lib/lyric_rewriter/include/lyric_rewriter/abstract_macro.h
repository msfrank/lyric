#ifndef LYRIC_REWRITER_ABSTRACT_MACRO_H
#define LYRIC_REWRITER_ABSTRACT_MACRO_H

#include <lyric_parser/archetype_state.h>

#include "macro_block.h"

namespace lyric_rewriter {

    class AbstractMacro {
    public:
        virtual ~AbstractMacro() = default;

        virtual tempo_utils::Status rewriteBlock(
            const lyric_parser::ArchetypeNode *macroCallNode,
            MacroBlock &macroBlock,
            lyric_parser::ArchetypeState *state) = 0;
    };
}

#endif // LYRIC_REWRITER_ABSTRACT_MACRO_H
