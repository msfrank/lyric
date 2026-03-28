#ifndef LYRIC_IMPORTER_EXISTENTIAL_IMPORT_H
#define LYRIC_IMPORTER_EXISTENTIAL_IMPORT_H

#include "base_import.h"
#include "module_import.h"

namespace lyric_importer {

    class ExistentialImport : public BaseImport {
    public:
        ExistentialImport(std::weak_ptr<ModuleImport> moduleImport, tu_uint32 existentialOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isDeclOnly();
        lyric_object::DeriveType getDerive();
        bool isHidden();

        std::weak_ptr<TypeImport> getExistentialType();
        lyric_common::SymbolUrl getSuperExistential();

        bool hasExistentialTemplate();
        std::weak_ptr<TemplateImport> getExistentialTemplate();

        lyric_common::SymbolUrl getMethod(std::string_view name);
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator methodsBegin();
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator methodsEnd();
        tu_uint8 numMethods();

        absl::flat_hash_map<lyric_common::TypeDef,std::weak_ptr<ImplImport>>::const_iterator implsBegin();
        absl::flat_hash_map<lyric_common::TypeDef,std::weak_ptr<ImplImport>>::const_iterator implsEnd();
        tu_uint8 numImpls();

        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin();
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd();
        int numSealedTypes();

    private:
        tu_uint32 m_existentialOffset;
        absl::Mutex m_lock;

        struct Priv {
            lyric_common::SymbolUrl symbolUrl;
            bool isDeclOnly = false;
            lyric_object::DeriveType derive = lyric_object::DeriveType::Invalid;
            bool isHidden = false;
            std::weak_ptr<TypeImport> existentialType;
            bool hasTemplate = false;
            std::weak_ptr<TemplateImport> existentialTemplate;
            lyric_common::SymbolUrl superExistential;
            absl::flat_hash_map<std::string,lyric_common::SymbolUrl> methods;
            absl::flat_hash_map<lyric_common::TypeDef,std::weak_ptr<ImplImport>> impls;
            absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        };
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_EXISTENTIAL_IMPORT_H
