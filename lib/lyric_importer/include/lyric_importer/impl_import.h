#ifndef LYRIC_IMPORTER_IMPL_IMPORT_H
#define LYRIC_IMPORTER_IMPL_IMPORT_H

#include "importer_types.h"
#include "module_import.h"

namespace lyric_importer {

    class ImplImport {
    public:
        ImplImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 implOffset);

        TypeImport *getImplType();
        lyric_common::SymbolUrl getImplConcept();
        lyric_common::SymbolUrl getReceiverUrl();

        absl::flat_hash_map<std::string,Extension>::const_iterator extensionsBegin();
        absl::flat_hash_map<std::string,Extension>::const_iterator extensionsEnd();
        int numExtensions();

    private:
        std::shared_ptr<ModuleImport> m_moduleImport;
        tu_uint32 m_implOffset;
        absl::Mutex m_lock;

        struct Priv;
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_IMPL_IMPORT_H
