#ifndef LYRIC_ANALYZER_STRUCT_ANALYZER_CONTEXT_H
#define LYRIC_ANALYZER_STRUCT_ANALYZER_CONTEXT_H

#include <lyric_assembler/struct_symbol.h>

#include "abstract_analyzer_context.h"
#include "analyzer_scan_driver.h"

namespace lyric_analyzer {

    class StructAnalyzerContext : public AbstractAnalyzerContext {
    public:
        StructAnalyzerContext(
            AnalyzerScanDriver *driver,
            lyric_assembler::StructSymbol *structSymbol);

        lyric_assembler::BlockHandle *getBlock() const override;

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status declareCtor(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status declareMember(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status declareMethod(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status declareImpl(const lyric_parser::ArchetypeNode *node);

    private:
        AnalyzerScanDriver *m_driver;
        lyric_assembler::StructSymbol *m_structSymbol;
        bool m_missingInit;
    };
}

#endif // LYRIC_ANALYZER_STRUCT_ANALYZER_CONTEXT_H
