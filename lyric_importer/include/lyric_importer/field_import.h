#ifndef LYRIC_IMPORTER_FIELD_IMPORT_H
#define LYRIC_IMPORTER_FIELD_IMPORT_H

#include "module_import.h"

namespace lyric_importer {

    class FieldImport {
    public:
        FieldImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 fieldOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        lyric_object::AccessType getAccess();
        bool isVariable();
        TypeImport *getFieldType();

        lyric_common::SymbolUrl getInitializer();

    private:
        std::shared_ptr<ModuleImport> m_moduleImport;
        tu_uint32 m_fieldOffset;
        absl::Mutex m_lock;

        struct Priv;
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_FIELD_IMPORT_H
