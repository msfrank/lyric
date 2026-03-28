#ifndef LYRIC_IMPORTER_BINDING_IMPORT_H
#define LYRIC_IMPORTER_BINDING_IMPORT_H

#include "base_import.h"
#include "module_import.h"

namespace lyric_importer {

    class BindingImport : public BaseImport {
    public:
        BindingImport(std::weak_ptr<ModuleImport> moduleImport, tu_uint32 bindingOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isHidden();

        std::weak_ptr<TypeImport> getBindingType();
        std::weak_ptr<TypeImport> getTargetType();

        bool hasBindingTemplate();
        std::weak_ptr<TemplateImport> getBindingTemplate();

    private:
        tu_uint32 m_bindingOffset;
        absl::Mutex m_lock;

        struct Priv {
            lyric_common::SymbolUrl symbolUrl;
            bool isHidden = false;
            std::weak_ptr<TypeImport> bindingType;
            bool hasTemplate = false;
            std::weak_ptr<TemplateImport> bindingTemplate;
            std::weak_ptr<TypeImport> targetType;
        };
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_BINDING_IMPORT_H
