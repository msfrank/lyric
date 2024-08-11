#ifndef LYRIC_ANALYZER_CONCEPT_ANALYZER_CONTEXT_H
#define LYRIC_ANALYZER_CONCEPT_ANALYZER_CONTEXT_H

#include <lyric_assembler/concept_symbol.h>

#include "abstract_analyzer_context.h"
#include "analyzer_scan_driver.h"

namespace lyric_analyzer {

    class ConceptAnalyzerContext : public AbstractAnalyzerContext {
    public:
        ConceptAnalyzerContext(
            AnalyzerScanDriver *driver,
            lyric_assembler::ConceptSymbol *conceptSymbol);

        lyric_assembler::BlockHandle *getBlock() const override;

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status declareAction(const lyric_parser::ArchetypeNode *node);

    private:
        AnalyzerScanDriver *m_driver;
        lyric_assembler::ConceptSymbol *m_conceptSymbol;
    };
}

#endif // LYRIC_ANALYZER_CONCEPT_ANALYZER_CONTEXT_H
