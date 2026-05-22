#ifndef LYRIC_ANALYZER_IMPL_ANALYZER_CONTEXT_H
#define LYRIC_ANALYZER_IMPL_ANALYZER_CONTEXT_H

#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/impl_handle.h>

#include "abstract_analyzer_context.h"
#include "analyzer_scan_driver.h"

namespace lyric_analyzer {

    class ImplAnalyzerContext : public AbstractAnalyzerContext {
    public:
        ImplAnalyzerContext(
            AnalyzerScanDriver *driver,
            lyric_assembler::ImplHandle *implHandle,
            const lyric_common::TypeDef &implType);

        lyric_assembler::BlockHandle *getBlock() const override;

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status processAliases(const lyric_parser::ArchetypeNode *node);

        tempo_utils::Result<lyric_assembler::BindingSymbol *> declareAlias(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status declareExtension(const lyric_parser::ArchetypeNode *node);

    private:
        AnalyzerScanDriver *m_driver;
        lyric_assembler::ImplHandle *m_implHandle;
        lyric_common::TypeDef m_implType;
        bool m_finalized = false;
    };
}

#endif // LYRIC_ANALYZER_IMPL_ANALYZER_CONTEXT_H
