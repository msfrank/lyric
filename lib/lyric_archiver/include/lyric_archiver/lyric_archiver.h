#ifndef LYRIC_ARCHIVER_LYRIC_ARCHIVER_H
#define LYRIC_ARCHIVER_LYRIC_ARCHIVER_H

#include <absl/container/flat_hash_map.h>

#include <lyric_assembler/object_state.h>
#include <lyric_common/module_location.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/module_cache.h>
#include <lyric_object/lyric_object.h>
#include <lyric_runtime/abstract_loader.h>
#include <tempo_tracing/trace_recorder.h>

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
            const lyric_common::ModuleLocation &origin,
            std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
            std::shared_ptr<tempo_tracing::TraceRecorder> recorder,
            const ArchiverOptions &options);

        tempo_utils::Status initialize();

        tempo_utils::Status importModule(const lyric_common::ModuleLocation &location);

        tempo_utils::Status insertModule(
            const lyric_common::ModuleLocation &location,
            const lyric_object::LyricObject &object);

        tempo_utils::Result<lyric_common::SymbolUrl> archiveSymbol(const lyric_common::SymbolUrl &symbolUrl);

        tempo_utils::Result<lyric_assembler::BindingSymbol *> declareBinding(
            const std::string &name,
            lyric_object::AccessType access,
            const std::vector<lyric_object::TemplateParameter> &templateParameters = {});

        tempo_utils::Result<lyric_object::LyricObject> toObject() const;

    private:
        lyric_common::ModuleLocation m_location;
        lyric_common::ModuleLocation m_origin;
        std::shared_ptr<lyric_importer::ModuleCache> m_localModuleCache;
        std::shared_ptr<lyric_importer::ModuleCache> m_systemModuleCache;
        std::shared_ptr<lyric_importer::ShortcutResolver> m_shortcutResolver;
        std::shared_ptr<tempo_tracing::TraceRecorder> m_recorder;
        ArchiverOptions m_options;

        std::unique_ptr<ArchiverState> m_archiverState;
    };
}

#endif // LYRIC_ARCHIVER_LYRIC_ARCHIVER_H
