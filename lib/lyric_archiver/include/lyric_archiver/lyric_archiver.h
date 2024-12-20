#ifndef LYRIC_ARCHIVER_LYRIC_ARCHIVER_H
#define LYRIC_ARCHIVER_LYRIC_ARCHIVER_H

#include <absl/container/flat_hash_map.h>

#include <lyric_assembler/object_state.h>
#include <lyric_common/module_location.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/module_cache.h>
#include <lyric_object/lyric_object.h>
#include <lyric_runtime/abstract_loader.h>
#include <tempo_tracing/scope_manager.h>

#include "archiver_state.h"

namespace lyric_archiver {

    /**
     *
     */
    struct ArchiverOptions {

        lyric_common::ModuleLocation preludeLocation;
    };

    class LyricArchiver {

    public:
        LyricArchiver(
            const lyric_common::ModuleLocation &location,
            std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            std::shared_ptr<tempo_tracing::TraceRecorder> recorder,
            const ArchiverOptions &options);

        tempo_utils::Status initialize();

        tempo_utils::Status importModule(const lyric_common::ModuleLocation &location);

        tempo_utils::Status insertModule(
            const lyric_common::ModuleLocation &location,
            const lyric_object::LyricObject &object);

        tempo_utils::Status archiveSymbol(
            const lyric_common::ModuleLocation &srcLocation,
            const std::string &srcIdentifier,
            const std::string &dstIdentifier = {},
            lyric_object::AccessType access = lyric_object::AccessType::Public);

        tempo_utils::Status archiveSymbol(
            const lyric_common::SymbolUrl &srcUrl,
            const std::string &dstIdentifier,
            lyric_object::AccessType access = lyric_object::AccessType::Public);

        tempo_utils::Result<lyric_object::LyricObject> toObject() const;

    private:
        lyric_common::ModuleLocation m_location;
        std::shared_ptr<lyric_importer::ModuleCache> m_localModuleCache;
        std::shared_ptr<lyric_importer::ModuleCache> m_systemModuleCache;
        std::shared_ptr<tempo_tracing::TraceRecorder> m_recorder;
        ArchiverOptions m_options;

        std::unique_ptr<tempo_tracing::ScopeManager> m_scopeManager;
        std::unique_ptr<ArchiverState> m_archiverState;
    };
}

#endif // LYRIC_ARCHIVER_LYRIC_ARCHIVER_H
