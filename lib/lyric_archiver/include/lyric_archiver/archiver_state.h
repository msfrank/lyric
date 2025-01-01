#ifndef LYRIC_ARCHIVER_ARCHIVER_STATE_H
#define LYRIC_ARCHIVER_ARCHIVER_STATE_H

#include <absl/container/flat_hash_map.h>

#include <lyric_assembler/object_state.h>
#include <lyric_common/module_location.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/module_cache.h>
#include <lyric_object/lyric_object.h>
#include <lyric_runtime/abstract_loader.h>
#include <tempo_tracing/scope_manager.h>

namespace lyric_archiver {

    /**
     *
     */
    struct ArchiverStateOptions {

        lyric_common::ModuleLocation preludeLocation;
    };

    class ArchiverState {

    public:
        ArchiverState(
            std::unique_ptr<lyric_assembler::ObjectState> &&objectState,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            lyric_assembler::ObjectRoot *objectRoot);

        lyric_assembler::ObjectState *objectState();
        lyric_importer::ModuleCache *systemModuleCache();
        lyric_assembler::ObjectRoot *objectRoot();

        tempo_utils::Status importObject(const lyric_common::ModuleLocation &location);

        tempo_utils::Status insertObject(
            const lyric_common::ModuleLocation &location,
            const lyric_object::LyricObject &object);

        tempo_utils::Result<lyric_common::SymbolUrl> archiveSymbol(const lyric_common::SymbolUrl &symbolUrl);

        tempo_utils::Status putPendingProc(
            const lyric_common::SymbolUrl &importUrl,
            const lyric_object::LyricObject &object,
            const lyric_object::ProcHeader &header,
            const lyric_object::BytecodeIterator &code,
            lyric_assembler::ProcHandle *procHandle);

        tempo_utils::Status performFixups();

        tempo_utils::Result<lyric_assembler::BindingSymbol *> declareBinding(
            const std::string &name,
            lyric_object::AccessType access,
            const std::vector<lyric_object::TemplateParameter> &templateParameters = {});

        tempo_utils::Result<lyric_object::LyricObject> toObject() const;

    private:
        std::unique_ptr<lyric_assembler::ObjectState> m_objectState;
        std::shared_ptr<lyric_importer::ModuleCache> m_systemModuleCache;
        lyric_assembler::ObjectRoot *m_objectRoot;

        absl::flat_hash_map<
            lyric_common::ModuleLocation,
            std::shared_ptr<lyric_importer::ModuleImport>> m_moduleImports;
        absl::flat_hash_map<
            lyric_common::ModuleLocation,
            std::string> m_importHashes;

        struct PendingProc {
            lyric_object::LyricObject object;
            lyric_object::ProcHeader header;
            lyric_object::BytecodeIterator code;
            lyric_assembler::ProcHandle *procHandle;
        };
        absl::flat_hash_map<
            lyric_common::SymbolUrl,
            PendingProc> m_pendingProcs;
    };
}

#endif // LYRIC_ARCHIVER_ARCHIVER_STATE_H
