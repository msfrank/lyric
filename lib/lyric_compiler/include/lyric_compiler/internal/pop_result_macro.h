#ifndef LYRIC_COMPILER_INTERNAL_POP_RESULT_MACRO_H
#define LYRIC_COMPILER_INTERNAL_POP_RESULT_MACRO_H

#include <lyric_rewriter/abstract_macro.h>

namespace lyric_compiler::internal {

    class PopResultMacro : public lyric_rewriter::AbstractMacro {
    public:
        PopResultMacro();

        tempo_utils::Status rewritePragma(
            const lyric_parser::ArchetypeNode *pragmaNode,
            lyric_rewriter::PragmaContext &ctx,
            lyric_parser::ArchetypeState *state) override;

        tempo_utils::Status rewriteDefinition(
            const lyric_parser::ArchetypeNode *macroCallNode,
            lyric_parser::ArchetypeNode *definitionNode,
            lyric_parser::ArchetypeState *state) override;

        tempo_utils::Status rewriteBlock(
            const lyric_parser::ArchetypeNode *macroCallNode,
            lyric_rewriter::MacroBlock &macroBlock,
            lyric_parser::ArchetypeState *state) override;
    };
}

#endif // LYRIC_COMPILER_INTERNAL_POP_RESULT_MACRO_H
