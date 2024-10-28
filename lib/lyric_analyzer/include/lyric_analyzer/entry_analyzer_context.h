#ifndef LYRIC_ANALYZER_ENTRY_ANALYZER_CONTEXT_H
#define LYRIC_ANALYZER_ENTRY_ANALYZER_CONTEXT_H

#include <lyric_assembler/object_root.h>

#include "abstract_analyzer_context.h"
#include "analyzer_scan_driver.h"

namespace lyric_analyzer {

    class EntryAnalyzerContext : public AbstractAnalyzerContext {
    public:
        EntryAnalyzerContext(AnalyzerScanDriver *driver, lyric_assembler::ObjectRoot *root);

        lyric_assembler::BlockHandle *getBlock() const override;

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

    private:
        AnalyzerScanDriver *m_driver;
        lyric_assembler::ObjectRoot *m_root;
    };
}

#endif // LYRIC_ANALYZER_ENTRY_ANALYZER_CONTEXT_H
