#ifndef LYRIC_SYMBOLIZER_SYMBOLIZER_SCAN_DRIVER_H
#define LYRIC_SYMBOLIZER_SYMBOLIZER_SCAN_DRIVER_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_rewriter/abstract_scan_driver.h>

namespace lyric_symbolizer {

    class SymbolizerScanDriver : public lyric_rewriter::AbstractScanDriver {
    public:
        explicit SymbolizerScanDriver(lyric_assembler::AssemblyState *state);

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

    private:
        lyric_assembler::AssemblyState *m_state;
        lyric_assembler::UndeclaredSymbol *m_entry;
        std::vector<std::string> m_symbolPath;

        tempo_utils::Status declareStatic(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status declareImport(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status pushDefinition(
            const lyric_parser::ArchetypeNode *node,
            lyric_object::LinkageSection section);
        tempo_utils::Status popDefinition();
    };
}

#endif // LYRIC_SYMBOLIZER_SYMBOLIZER_SCAN_DRIVER_H
