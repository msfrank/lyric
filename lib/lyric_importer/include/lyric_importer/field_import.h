#ifndef LYRIC_IMPORTER_FIELD_IMPORT_H
#define LYRIC_IMPORTER_FIELD_IMPORT_H

#include "base_import.h"
#include "module_import.h"

namespace lyric_importer {

    class FieldImport : public BaseImport {
    public:
        FieldImport(std::weak_ptr<ModuleImport> moduleImport, tu_uint32 fieldOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isDeclOnly();
        bool isVariable();
        bool isHidden();

        std::weak_ptr<TypeImport> getFieldType();

        lyric_common::SymbolUrl getInitializer();

    private:
        tu_uint32 m_fieldOffset;
        absl::Mutex m_lock;

        struct Priv {
            lyric_common::SymbolUrl symbolUrl;
            bool isHidden = false;
            bool isVariable = false;
            bool isDeclOnly = false;
            std::weak_ptr<TypeImport> fieldType;
            lyric_common::SymbolUrl initializer;
        };
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_FIELD_IMPORT_H
