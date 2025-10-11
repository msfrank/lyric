#ifndef LYRIC_ANALYZER_IMPORT_ANALYZER_CONTEXT_H
#define LYRIC_ANALYZER_IMPORT_ANALYZER_CONTEXT_H

#include <lyric_assembler/concept_symbol.h>

#include "abstract_analyzer_context.h"
#include "analyzer_scan_driver.h"

namespace lyric_analyzer {

    class ImportAnalyzerContext : public AbstractAnalyzerContext {
    public:
        ImportAnalyzerContext(
            AnalyzerScanDriver *driver,
            lyric_assembler::BlockHandle *importBlock,
            const tempo_utils::Url &importLocation);

        lyric_assembler::BlockHandle *getBlock() const override;

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status addSymbol(const lyric_parser::ArchetypeNode *node);

    private:
        AnalyzerScanDriver *m_driver;
        lyric_assembler::BlockHandle *m_importBlock;
        tempo_utils::Url m_importLocation;
        absl::flat_hash_set<lyric_assembler::ImportRef> m_importRefs;
    };
}

#endif // LYRIC_ANALYZER_IMPORT_ANALYZER_CONTEXT_H