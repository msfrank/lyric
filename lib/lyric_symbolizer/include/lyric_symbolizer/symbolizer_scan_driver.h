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

        tempo_utils::Status declareTypename(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status declareStatic(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status declareImport(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status pushDefinition(
            const lyric_parser::ArchetypeNode *node,
            lyric_object::LinkageSection section);
        tempo_utils::Status popDefinition();

        tempo_utils::Status pushNamespace(const lyric_parser::ArchetypeNode *node);
        tempo_utils::Status putNamespaceTarget(const lyric_common::SymbolUrl &symbolUrl);
        tempo_utils::Status popNamespace();
    };

    class SymbolizerScanDriverBuilder : public lyric_rewriter::AbstractScanDriverBuilder {
    public:
        SymbolizerScanDriverBuilder(
            const lyric_common::ModuleLocation &location,
            const lyric_common::ModuleLocation &origin,
            std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
            const lyric_assembler::ObjectStateOptions &objectStateOptions);

        tempo_utils::Status applyPragma(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node) override;

        tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractScanDriver>> makeScanDriver() override;

        tempo_utils::Result<lyric_object::LyricObject> toObject() const;

    private:
        lyric_common::ModuleLocation m_location;
        lyric_common::ModuleLocation m_origin;
        std::shared_ptr<lyric_importer::ModuleCache> m_localModuleCache;
        std::shared_ptr<lyric_importer::ModuleCache> m_systemModuleCache;
        std::shared_ptr<lyric_importer::ShortcutResolver> m_shortcutResolver;
        lyric_assembler::ObjectStateOptions m_objectStateOptions;

        std::unique_ptr<lyric_assembler::ObjectState> m_state;
    };
}

#endif // LYRIC_SYMBOLIZER_SYMBOLIZER_SCAN_DRIVER_H
