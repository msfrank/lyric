#ifndef LYRIC_IMPORTER_INSTANCE_IMPORT_H
#define LYRIC_IMPORTER_INSTANCE_IMPORT_H

#include <lyric_common/type_def.h>

#include "importer_types.h"
#include "module_import.h"

namespace lyric_importer {

    class InstanceImport {
    public:
        InstanceImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 instanceOffset);

        lyric_common::SymbolUrl getSymbolUrl();
        bool isAbstract();
        lyric_object::DeriveType getDerive();
        TypeImport *getInstanceType();
        TemplateImport *getInstanceTemplate();
        lyric_common::SymbolUrl getSuperInstance();

        lyric_common::SymbolUrl getMember(std::string_view name);
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator membersBegin();
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator membersEnd();
        tu_uint8 numMembers();

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

        bool hasAllocator();
        tu_uint32 getAllocator();

    private:
        std::shared_ptr<ModuleImport> m_moduleImport;
        tu_uint32 m_instanceOffset;
        absl::Mutex m_lock;

        struct Priv;
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_INSTANCE_IMPORT_H
