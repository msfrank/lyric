#ifndef LYRIC_REWRITER_ABSTRACT_MACRO_H
#define LYRIC_REWRITER_ABSTRACT_MACRO_H

#include <lyric_parser/archetype_state.h>

#include "macro_block.h"
#include "pragma_context.h"

namespace lyric_rewriter {

    class AbstractMacro {
    public:
        virtual ~AbstractMacro() = default;

        virtual tempo_utils::Status rewritePragma(
            const lyric_parser::ArchetypeNode *pragmaNode,
            PragmaContext &ctx,
            lyric_parser::ArchetypeState *state) = 0;

        virtual tempo_utils::Status rewriteDefinition(
            const lyric_parser::ArchetypeNode *macroCallNode,
            lyric_parser::ArchetypeNode *definitionNode,
            lyric_parser::ArchetypeState *state) = 0;

        virtual tempo_utils::Status rewriteBlock(
            const lyric_parser::ArchetypeNode *macroCallNode,
            MacroBlock &macroBlock,
            lyric_parser::ArchetypeState *state) = 0;
    };
}

#endif // LYRIC_REWRITER_ABSTRACT_MACRO_H
