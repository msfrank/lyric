#ifndef LYRIC_SYMBOLIZER_SYMBOLIZER_SCAN_DRIVER_H
#define LYRIC_SYMBOLIZER_SYMBOLIZER_SCAN_DRIVER_H

#include <lyric_assembler/object_root.h>
#include <lyric_assembler/object_state.h>
#include <lyric_rewriter/abstract_scan_driver.h>

namespace lyric_symbolizer {

    class SymbolizerScanDriver : public lyric_rewriter::AbstractScanDriver {
    public:
        SymbolizerScanDriver(
            lyric_assembler::ObjectRoot *root,
            lyric_assembler::ObjectState *state);

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status finish() override;

    private:
        lyric_assembler::ObjectRoot *m_root;
        lyric_assembler::ObjectState *m_state;
        std::vector<std::string> m_symbolPath;
        std::stack<lyric_assembler::NamespaceSymbol *> m_namespaces;

        tempo_utils::Status declareStatic(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status declareImport(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status pushDefinition(
            const lyric_parser::ArchetypeNode *node,
            lyric_object::LinkageSection section);
        tempo_utils::Status popDefinition();

        tempo_utils::Status pushNamespace(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status putNamespaceBinding(
            const std::string &name,
            const lyric_common::SymbolUrl &symbolUrl,
            lyric_object::AccessType access);
        tempo_utils::Status popNamespace();
    };
}

#endif // LYRIC_SYMBOLIZER_SYMBOLIZER_SCAN_DRIVER_H
