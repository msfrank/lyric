#ifndef LYRIC_IMPORTER_EXISTENTIAL_IMPORT_H
#define LYRIC_IMPORTER_EXISTENTIAL_IMPORT_H

#include "module_import.h"

namespace lyric_importer {

    class ExistentialImport {
    public:
        ExistentialImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 existentialOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isDeclOnly();
        lyric_object::DeriveType getDerive();
        lyric_object::AccessType getAccess();

        TypeImport *getExistentialType();
        TemplateImport *getExistentialTemplate();
        lyric_common::SymbolUrl getSuperExistential();

        lyric_common::SymbolUrl getMethod(std::string_view name);
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator methodsBegin();
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator methodsEnd();
        tu_uint8 numMethods();

        absl::flat_hash_map<lyric_common::TypeDef,ImplImport *>::const_iterator implsBegin();
        absl::flat_hash_map<lyric_common::TypeDef,ImplImport *>::const_iterator implsEnd();
        tu_uint8 numImpls();

        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin();
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd();
        int numSealedTypes();

    private:
        std::shared_ptr<ModuleImport> m_moduleImport;
        tu_uint32 m_existentialOffset;
        absl::Mutex m_lock;

        struct Priv;
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_EXISTENTIAL_IMPORT_H
