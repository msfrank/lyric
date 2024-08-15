#ifndef LYRIC_ANALYZER_ANALYZER_SCAN_DRIVER_H
#define LYRIC_ANALYZER_ANALYZER_SCAN_DRIVER_H

#include <lyric_assembler/object_state.h>
#include <lyric_rewriter/abstract_scan_driver.h>
#include <lyric_typing/type_system.h>

#include "abstract_analyzer_context.h"

namespace lyric_analyzer {

    class AnalyzerScanDriver : public lyric_rewriter::AbstractScanDriver {
    public:
        explicit AnalyzerScanDriver(lyric_assembler::ObjectState *state);
        ~AnalyzerScanDriver() override;

        tempo_utils::Status initialize();

        tempo_utils::Status arrange(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &children) override;

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

        lyric_typing::TypeSystem *getTypeSystem() const;

        AbstractAnalyzerContext *peekContext();
        tempo_utils::Status pushContext(std::unique_ptr<AbstractAnalyzerContext> ctx);
        tempo_utils::Status popContext();

        tempo_utils::Status declareStatic(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
        tempo_utils::Status pushFunction(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
        tempo_utils::Status pushNamespace(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
        tempo_utils::Status pushClass(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);
        tempo_utils::Status pushConcept(const lyric_parser::ArchetypeNode *node, lyric_assembler::BlockHandle *block);

    private:
        lyric_assembler::ObjectState *m_state;
        lyric_assembler::NamespaceSymbol *m_root;
        lyric_assembler::CallSymbol *m_entry;
        lyric_typing::TypeSystem *m_typeSystem;
        std::vector<std::unique_ptr<AbstractAnalyzerContext>> m_stack;
    };
}

#endif // LYRIC_ANALYZER_ANALYZER_SCAN_DRIVER_H
