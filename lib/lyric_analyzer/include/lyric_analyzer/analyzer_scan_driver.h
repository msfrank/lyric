#ifndef LYRIC_ANALYZER_ANALYZER_SCAN_DRIVER_H
#define LYRIC_ANALYZER_ANALYZER_SCAN_DRIVER_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_rewriter/abstract_scan_driver.h>
#include <lyric_typing/type_system.h>

namespace lyric_analyzer {

    class AnalyzerScanDriver : public lyric_rewriter::AbstractScanDriver {
    public:
        explicit AnalyzerScanDriver(lyric_assembler::AssemblyState *state);
        ~AnalyzerScanDriver() override;

        tempo_utils::Status initialize();

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

    private:
        lyric_assembler::AssemblyState *m_state;
        lyric_assembler::NamespaceSymbol *m_root;
        lyric_assembler::CallSymbol *m_entry;
        lyric_typing::TypeSystem *m_typeSystem;
        std::vector<lyric_assembler::BlockHandle *> m_blocks;

        tempo_utils::Status declareStatic(const lyric_parser::ArchetypeNode *node, lyric_parser::BindingType binding);
        tempo_utils::Status pushDefinition(
            const lyric_parser::ArchetypeNode *node,
            lyric_object::LinkageSection section);
        tempo_utils::Status popDefinition();
    };
}

#endif // LYRIC_ANALYZER_ANALYZER_SCAN_DRIVER_H
