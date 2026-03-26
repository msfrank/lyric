#ifndef LYRIC_IMPORTER_STATIC_IMPORT_H
#define LYRIC_IMPORTER_STATIC_IMPORT_H

#include "base_import.h"
#include "module_import.h"

namespace lyric_importer {

    class StaticImport : public BaseImport {
    public:
        StaticImport(std::weak_ptr<ModuleImport> moduleImport, tu_uint32 staticOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isVariable();
        bool isDeclOnly();
        bool isHidden();

        TypeImport *getStaticType();
        lyric_common::SymbolUrl getInitializer();

    private:
        tu_uint32 m_staticOffset;
        absl::Mutex m_lock;

        struct Priv {
            lyric_common::SymbolUrl symbolUrl;
            bool isVariable = false;
            bool isDeclOnly = false;
            bool isHidden = false;
            TypeImport *staticType = nullptr;
            lyric_common::SymbolUrl initializer;
        };
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_STATIC_IMPORT_H
