#ifndef LYRIC_ANALYZER_CLASS_ANALYZER_CONTEXT_H
#define LYRIC_ANALYZER_CLASS_ANALYZER_CONTEXT_H

#include <lyric_assembler/class_symbol.h>

#include "abstract_analyzer_context.h"
#include "analyzer_scan_driver.h"

namespace lyric_analyzer {

    class ClassAnalyzerContext : public AbstractAnalyzerContext {
    public:
        ClassAnalyzerContext(
            AnalyzerScanDriver *driver,
            lyric_assembler::ClassSymbol *classSymbol,
            const lyric_parser::ArchetypeNode *initNode);

        lyric_assembler::BlockHandle *getBlock() const override;

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status declareMember(const lyric_parser::ArchetypeNode *node, bool isVariable);
        tempo_utils::Status declareMethod(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status declareImpl(const lyric_parser::ArchetypeNode *node);

    private:
        AnalyzerScanDriver *m_driver;
        lyric_assembler::ClassSymbol *m_classSymbol;
        const lyric_parser::ArchetypeNode *m_initNode;
    };
}

#endif // LYRIC_ANALYZER_CLASS_ANALYZER_CONTEXT_H
