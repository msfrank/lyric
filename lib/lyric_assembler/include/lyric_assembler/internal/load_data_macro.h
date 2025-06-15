#ifndef LYRIC_ASSEMBLER_INTERNAL_LOAD_DATA_MACRO_H
#define LYRIC_ASSEMBLER_INTERNAL_LOAD_DATA_MACRO_H

#include <lyric_rewriter/abstract_macro.h>

namespace lyric_assembler::internal {

    class LoadDataMacro : public lyric_rewriter::AbstractMacro {
    public:
        LoadDataMacro();

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

#endif // LYRIC_ASSEMBLER_INTERNAL_LOAD_DATA_MACRO_H
