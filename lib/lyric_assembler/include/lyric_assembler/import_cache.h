#ifndef LYRIC_ASSEMBLER_IMPORT_CACHE_H
#define LYRIC_ASSEMBLER_IMPORT_CACHE_H

#include <lyric_common/module_location.h>
#include <lyric_importer/module_cache.h>

#include "assembler_tracer.h"
#include "assembler_types.h"
#include "base_symbol.h"
#include "block_handle.h"
#include "symbol_cache.h"

namespace lyric_assembler {

    /**
     *
     */
    class ImportCache {
    public:
        explicit ImportCache(
            lyric_assembler::ObjectState *state,
            std::shared_ptr<lyric_runtime::AbstractLoader> loader,
            std::shared_ptr<lyric_importer::ModuleCache> sharedModuleCache,
            SymbolCache *symbolCache,
            AssemblerTracer *tracer);
        ~ImportCache();

        tempo_utils::Result<std::shared_ptr<lyric_importer::ModuleImport>> importModule(
            const lyric_common::ModuleLocation &importLocation,
            ImportFlags importFlags);

        tempo_utils::Status importModule(
            const lyric_common::ModuleLocation &importLocation,
            BlockHandle *block,
            const absl::flat_hash_set<ImportRef> &importRefs = {},
            bool preload = false);

        std::shared_ptr<lyric_importer::ModuleImport> getModule(const lyric_common::ModuleLocation &importLocation);

        tempo_utils::Result<ActionSymbol *> importAction(const lyric_common::SymbolUrl &actionUrl);
        tempo_utils::Result<CallSymbol *> importCall(const lyric_common::SymbolUrl &callUrl);
        tempo_utils::Result<ClassSymbol *> importClass(const lyric_common::SymbolUrl &classUrl);
        tempo_utils::Result<ConceptSymbol *> importConcept(const lyric_common::SymbolUrl &conceptUrl);
        tempo_utils::Result<EnumSymbol *> importEnum(const lyric_common::SymbolUrl &enumUrl);
        tempo_utils::Result<ExistentialSymbol *> importExistential(const lyric_common::SymbolUrl &existentialUrl);
        tempo_utils::Result<FieldSymbol *> importField(const lyric_common::SymbolUrl &fieldUrl);
        tempo_utils::Result<InstanceSymbol *> importInstance(const lyric_common::SymbolUrl &instanceUrl);
        tempo_utils::Result<NamespaceSymbol *> importNamespace(const lyric_common::SymbolUrl &namespaceUrl);
        tempo_utils::Result<StaticSymbol *> importStatic(const lyric_common::SymbolUrl &staticUrl);
        tempo_utils::Result<StructSymbol *> importStruct(const lyric_common::SymbolUrl &structUrl);

        tempo_utils::Result<AbstractSymbol *> importSymbol(const lyric_common::SymbolUrl &symbolUrl);

        bool hasImport(const lyric_common::ModuleLocation &importLocation) const;
        ImportHandle *getImport(const lyric_common::ModuleLocation &importLocation) const;
        absl::flat_hash_map<lyric_common::ModuleLocation,ImportHandle *>::const_iterator importsBegin() const;
        absl::flat_hash_map<lyric_common::ModuleLocation,ImportHandle *>::const_iterator importsEnd() const;
        int numImports() const;

        tempo_utils::Status insertImport(
            const lyric_common::ModuleLocation &importLocation,
            ImportFlags importFlags);

    private:
        lyric_assembler::ObjectState *m_state;
        std::shared_ptr<lyric_runtime::AbstractLoader> m_loader;
        std::shared_ptr<lyric_importer::ModuleCache> m_privateModuleCache;
        std::shared_ptr<lyric_importer::ModuleCache> m_sharedModuleCache;
        SymbolCache *m_symbolCache;
        AssemblerTracer *m_tracer;
        absl::flat_hash_map<lyric_common::ModuleLocation, ImportHandle *> m_importcache;
    };
}

#endif // LYRIC_ASSEMBLER_IMPORT_CACHE_H
