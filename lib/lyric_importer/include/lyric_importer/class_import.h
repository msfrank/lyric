#ifndef LYRIC_IMPORTER_CLASS_IMPORT_H
#define LYRIC_IMPORTER_CLASS_IMPORT_H

#include <lyric_common/type_def.h>

#include "base_import.h"
#include "module_import.h"

namespace lyric_importer {

    class ClassImport : public BaseImport {
    public:
        ClassImport(std::weak_ptr<ModuleImport> moduleImport, tu_uint32 classOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isDeclOnly();
        lyric_object::DeriveType getDerive();
        bool isHidden();

        std::weak_ptr<TypeImport> getClassType();
        lyric_common::SymbolUrl getSuperClass();

        bool hasClassTemplate();
        std::weak_ptr<TemplateImport> getClassTemplate();

        lyric_common::SymbolUrl getMember(std::string_view name);
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator membersBegin();
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator membersEnd();
        tu_uint8 numMembers();

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

        bool hasAllocator();
        std::string getAllocator();

    private:
        tu_uint32 m_classOffset;
        absl::Mutex m_lock;

        struct Priv {
            lyric_common::SymbolUrl symbolUrl;
            bool isDeclOnly = false;
            lyric_object::DeriveType derive = lyric_object::DeriveType::Invalid;
            bool isHidden = false;
            std::weak_ptr<TypeImport> classType;
            bool hasTemplate = false;
            std::weak_ptr<TemplateImport> classTemplate;
            lyric_common::SymbolUrl superClass;
            absl::flat_hash_map<std::string,lyric_common::SymbolUrl> members;
            absl::flat_hash_map<std::string,lyric_common::SymbolUrl> methods;
            absl::flat_hash_map<lyric_common::TypeDef,std::weak_ptr<ImplImport>> impls;
            absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
            std::string allocator;
        };
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_CLASS_IMPORT_H
