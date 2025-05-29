#ifndef LYRIC_COMPILER_COMPILER_SCAN_DRIVER_H
#define LYRIC_COMPILER_COMPILER_SCAN_DRIVER_H

#include <lyric_assembler/object_state.h>
#include <lyric_rewriter/abstract_scan_driver.h>
#include <lyric_typing/type_system.h>

#include "visitor_context.h"

namespace lyric_compiler {

//    // forward declarations
//    class BaseGrouping;
//    class EnterContext;

    class CompilerScanDriver : public lyric_rewriter::AbstractScanDriver {
    public:
        CompilerScanDriver(lyric_assembler::ObjectRoot *root, lyric_assembler::ObjectState *state);
        ~CompilerScanDriver() override;

        tempo_utils::Status initialize(std::unique_ptr<BaseGrouping> &&rootGrouping);

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status finish() override;

        lyric_assembler::ObjectRoot *getObjectRoot() const;
        lyric_assembler::ObjectState *getState() const;
        lyric_assembler::FundamentalCache *getFundamentalCache() const;
        lyric_assembler::ImplCache *getImplCache() const;
        lyric_assembler::ImportCache *getImportCache() const;
        lyric_assembler::LiteralCache *getLiteralCache() const;
        lyric_assembler::SymbolCache *getSymbolCache() const;
        lyric_assembler::TypeCache *getTypeCache() const;
        lyric_typing::TypeSystem *getTypeSystem() const;

        BaseGrouping *peekGrouping();
        tempo_utils::Status popGrouping();
        tu_uint32 numGroupings() const;

        lyric_common::TypeDef peekResult();
        tempo_utils::Status pushResult(const lyric_common::TypeDef &result);
        tempo_utils::Status popResult();
        tu_uint32 numResults() const;

    private:
        lyric_assembler::ObjectRoot *m_root;
        lyric_assembler::ObjectState *m_state;
        lyric_typing::TypeSystem *m_typeSystem;
        std::stack<lyric_common::TypeDef> m_results;

        struct GroupingData {
            std::unique_ptr<BaseGrouping> grouping;
            const lyric_parser::ArchetypeNode *node = nullptr;
            bool pending = true;
        };
        std::vector<std::unique_ptr<GroupingData>> m_groupings;

        friend class EnterContext;
        friend class ExitContext;
    };

    class CompilerScanDriverBuilder : public lyric_rewriter::AbstractScanDriverBuilder {
    public:
        CompilerScanDriverBuilder(
            const lyric_common::ModuleLocation &location,
            std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
            tempo_tracing::ScopeManager *scopeManager,
            const lyric_assembler::ObjectStateOptions &objectStateOptions);

        tempo_utils::Status applyPragma(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node) override;

        tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractScanDriver>> makeScanDriver() override;

        tempo_utils::Result<lyric_object::LyricObject> toObject() const;

    private:
        lyric_common::ModuleLocation m_location;
        std::shared_ptr<lyric_importer::ModuleCache> m_localModuleCache;
        std::shared_ptr<lyric_importer::ModuleCache> m_systemModuleCache;
        std::shared_ptr<lyric_importer::ShortcutResolver> m_shortcutResolver;
        tempo_tracing::ScopeManager *m_scopeManager;
        lyric_assembler::ObjectStateOptions m_objectStateOptions;

        lyric_common::ModuleLocation m_pluginLocation;

        std::unique_ptr<lyric_assembler::ObjectState> m_state;
    };
}

#endif // LYRIC_COMPILER_COMPILER_SCAN_DRIVER_H
