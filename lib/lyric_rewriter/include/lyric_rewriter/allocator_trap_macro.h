#ifndef LYRIC_REWRITER_ALLOCATOR_TRAP_MACRO_H
#define LYRIC_REWRITER_ALLOCATOR_TRAP_MACRO_H

#include "abstract_macro.h"

namespace lyric_rewriter {

    class AllocatorTrapMacro : public AbstractMacro {
    public:
        AllocatorTrapMacro();

        tempo_utils::Status rewriteDefinition(
            const lyric_parser::ArchetypeNode *macroCallNode,
            lyric_parser::ArchetypeNode *definitionNode,
            lyric_parser::ArchetypeState *state) override;

        tempo_utils::Status rewriteBlock(
            const lyric_parser::ArchetypeNode *macroCallNode,
            MacroBlock &macroBlock,
            lyric_parser::ArchetypeState *state) override;
    };
}

#endif // LYRIC_REWRITER_ALLOCATOR_TRAP_MACRO_H
