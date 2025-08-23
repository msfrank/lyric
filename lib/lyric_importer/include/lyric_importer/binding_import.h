#ifndef LYRIC_IMPORTER_BINDING_IMPORT_H
#define LYRIC_IMPORTER_BINDING_IMPORT_H

#include "base_import.h"
#include "module_import.h"

namespace lyric_importer {

    class BindingImport : public BaseImport {
    public:
        BindingImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 bindingOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isHidden();
        TypeImport *getBindingType();
        TemplateImport *getBindingTemplate();
        TypeImport *getTargetType();

    private:
        tu_uint32 m_bindingOffset;
        absl::Mutex m_lock;

        struct Priv;
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_BINDING_IMPORT_H
