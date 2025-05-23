#ifndef LYRIC_IMPORTER_NAMESPACE_IMPORT_H
#define LYRIC_IMPORTER_NAMESPACE_IMPORT_H

#include "base_import.h"
#include "module_import.h"

namespace lyric_importer {

    class NamespaceImport : public BaseImport {
    public:
        NamespaceImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 namespaceOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isDeclOnly();
        lyric_object::AccessType getAccess();

        lyric_common::SymbolUrl getSuperNamespace();

        absl::flat_hash_set<lyric_common::SymbolUrl>::const_iterator symbolsBegin();
        absl::flat_hash_set<lyric_common::SymbolUrl>::const_iterator symbolsEnd();
        int numBindings();

    private:
        tu_uint32 m_namespaceOffset;
        absl::Mutex m_lock;

        struct Priv;
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_NAMESPACE_IMPORT_H
