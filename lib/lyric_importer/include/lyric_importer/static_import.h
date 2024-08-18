#ifndef LYRIC_IMPORTER_STATIC_IMPORT_H
#define LYRIC_IMPORTER_STATIC_IMPORT_H

#include "module_import.h"

namespace lyric_importer {

    class StaticImport {
    public:
        StaticImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 staticOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isVariable();
        bool isDeclOnly();
        lyric_object::AccessType getAccess();

        TypeImport *getStaticType();
        lyric_common::SymbolUrl getInitializer();

    private:
        std::shared_ptr<ModuleImport> m_moduleImport;
        tu_uint32 m_staticOffset;
        absl::Mutex m_lock;

        struct Priv;
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_STATIC_IMPORT_H
