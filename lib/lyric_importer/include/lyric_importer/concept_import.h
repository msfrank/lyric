#ifndef LYRIC_IMPORTER_CONCEPT_IMPORT_H
#define LYRIC_IMPORTER_CONCEPT_IMPORT_H

#include <lyric_common/type_def.h>

#include "base_import.h"
#include "module_import.h"

namespace lyric_importer {

    class ConceptImport : public BaseImport {
    public:
        ConceptImport(std::weak_ptr<ModuleImport> moduleImport, tu_uint32 conceptOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isDeclOnly();
        lyric_object::DeriveType getDerive();
        bool isHidden();

        std::weak_ptr<TypeImport> getConceptType();
        lyric_common::SymbolUrl getSuperConcept();

        bool hasConceptTemplate();
        std::weak_ptr<TemplateImport> getConceptTemplate();

        lyric_common::SymbolUrl getAction(std::string_view name);
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator actionsBegin();
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator actionsEnd();
        tu_uint8 numActions();

        absl::flat_hash_map<lyric_common::TypeDef,std::weak_ptr<ImplImport>>::const_iterator implsBegin();
        absl::flat_hash_map<lyric_common::TypeDef,std::weak_ptr<ImplImport>>::const_iterator implsEnd();
        tu_uint8 numImpls();

        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin();
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd();
        int numSealedTypes();

    private:
        tu_uint32 m_conceptOffset;
        absl::Mutex m_lock;

        struct Priv {
            lyric_common::SymbolUrl symbolUrl;
            bool isDeclOnly = false;
            lyric_object::DeriveType derive = lyric_object::DeriveType::Invalid;
            bool isHidden = false;
            std::weak_ptr<TypeImport> conceptType;
            bool hasTemplate = false;
            std::weak_ptr<TemplateImport> conceptTemplate;
            lyric_common::SymbolUrl superConcept;
            absl::flat_hash_map<std::string,lyric_common::SymbolUrl> actions;
            absl::flat_hash_map<lyric_common::TypeDef,std::weak_ptr<ImplImport>> impls;
            absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        };
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_CONCEPT_IMPORT_H
