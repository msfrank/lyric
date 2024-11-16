#ifndef LYRIC_REWRITER_PUSH_RESULT_MACRO_H
#define LYRIC_REWRITER_PUSH_RESULT_MACRO_H

#include "abstract_macro.h"

namespace lyric_rewriter {

    class PushResultMacro : public AbstractMacro {
    public:
        PushResultMacro();

        tempo_utils::Status rewriteBlock(
            const lyric_parser::ArchetypeNode *macroCallNode,
            MacroBlock &macroBlock,
            lyric_parser::ArchetypeState *state) override;
    };
}

#endif // LYRIC_REWRITER_PUSH_RESULT_MACRO_H
