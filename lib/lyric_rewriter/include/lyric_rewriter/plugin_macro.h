#ifndef LYRIC_REWRITER_PLUGIN_MACRO_H
#define LYRIC_REWRITER_PLUGIN_MACRO_H

#include "abstract_macro.h"

namespace lyric_rewriter {

    class PluginMacro : public AbstractMacro {
    public:
        PluginMacro();

        tempo_utils::Status rewritePragma(
            const lyric_parser::ArchetypeNode *pragmaNode,
            PragmaContext &ctx,
            lyric_parser::ArchetypeState *state) override;

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

#endif // LYRIC_REWRITER_PLUGIN_MACRO_H
