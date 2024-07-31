#ifndef LYRIC_REWRITER_MACRO_REWRITE_DRIVER_H
#define LYRIC_REWRITER_MACRO_REWRITE_DRIVER_H

#include "abstract_rewrite_driver.h"
#include "macro_registry.h"

namespace lyric_rewriter {

    class MacroRewriteDriver : public AbstractRewriteDriver {
    public:
        explicit MacroRewriteDriver(MacroRegistry *registry);

        tempo_utils::Status enter(
            lyric_parser::ArchetypeState *state,
            lyric_parser::ArchetypeNode *node,
            VisitorContext &ctx) override;

        tempo_utils::Status exit(
            lyric_parser::ArchetypeState *state,
            lyric_parser::ArchetypeNode *node,
            const VisitorContext &ctx) override;

        tempo_utils::Status rewriteMacroList(
            lyric_parser::ArchetypeNode *macroListNode,
            MacroBlock &macroBlock);

    private:
        MacroRegistry *m_registry;
        lyric_parser::ArchetypeNode *m_macroList;
    };
}

#endif // LYRIC_REWRITER_MACRO_REWRITE_DRIVER_H
