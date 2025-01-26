#ifndef LYRIC_IMPORTER_CONCEPT_IMPORT_H
#define LYRIC_IMPORTER_CONCEPT_IMPORT_H

#include <lyric_common/type_def.h>

#include "base_import.h"
#include "module_import.h"

namespace lyric_importer {

    class ConceptImport : public BaseImport {
    public:
        ConceptImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 conceptOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isDeclOnly();
        lyric_object::DeriveType getDerive();
        lyric_object::AccessType getAccess();

        TypeImport *getConceptType();
        TemplateImport *getConceptTemplate();
        lyric_common::SymbolUrl getSuperConcept();

        lyric_common::SymbolUrl getAction(std::string_view name);
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator actionsBegin();
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator actionsEnd();
        tu_uint8 numActions();

        absl::flat_hash_map<lyric_common::TypeDef,ImplImport *>::const_iterator implsBegin();
        absl::flat_hash_map<lyric_common::TypeDef,ImplImport *>::const_iterator implsEnd();
        tu_uint8 numImpls();

        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin();
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd();
        int numSealedTypes();

    private:
        tu_uint32 m_conceptOffset;
        absl::Mutex m_lock;

        struct Priv;
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_CONCEPT_IMPORT_H
