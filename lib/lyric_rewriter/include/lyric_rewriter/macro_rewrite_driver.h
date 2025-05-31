#ifndef LYRIC_REWRITER_MACRO_REWRITE_DRIVER_H
#define LYRIC_REWRITER_MACRO_REWRITE_DRIVER_H

#include "abstract_rewrite_driver.h"
#include "macro_registry.h"

namespace lyric_rewriter {

    class MacroRewriteDriver : public AbstractRewriteDriver {
    public:
        explicit MacroRewriteDriver(std::shared_ptr<MacroRegistry> registry);

        tempo_utils::Status enter(
            lyric_parser::ArchetypeState *state,
            lyric_parser::ArchetypeNode *node,
            VisitorContext &ctx) override;

        tempo_utils::Status exit(
            lyric_parser::ArchetypeState *state,
            lyric_parser::ArchetypeNode *node,
            const VisitorContext &ctx) override;

        tempo_utils::Status finish() override;

        tempo_utils::Status rewriteMacroDefinition(
            lyric_parser::ArchetypeNode *macroListNode,
            lyric_parser::ArchetypeNode *definitionNode,
            lyric_parser::ArchetypeState *state);

        tempo_utils::Status rewriteMacroBlock(
            lyric_parser::ArchetypeNode *macroListNode,
            MacroBlock &macroBlock);

    private:
        std::shared_ptr<MacroRegistry> m_registry;
        lyric_parser::ArchetypeNode *m_macroList;
    };

    class MacroRewriteDriverBuilder : public AbstractRewriteDriverBuilder {
    public:
        explicit MacroRewriteDriverBuilder(std::shared_ptr<MacroRegistry> registry);

        tempo_utils::Status rewritePragma(
            lyric_parser::ArchetypeState *state,
            lyric_parser::ArchetypeNode *node,
            PragmaContext &ctx) override;

        tempo_utils::Result<std::shared_ptr<AbstractRewriteDriver>> makeRewriteDriver() override;

    private:
        std::shared_ptr<MacroRegistry> m_registry;
    };
}

#endif // LYRIC_REWRITER_MACRO_REWRITE_DRIVER_H
