#ifndef LYRIC_IMPORTER_TEMPLATE_IMPORT_H
#define LYRIC_IMPORTER_TEMPLATE_IMPORT_H

#include "base_import.h"
#include "importer_types.h"
#include "module_import.h"

namespace lyric_importer {

    class TemplateImport : public BaseImport {
    public:
        TemplateImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 templateOffset);

        lyric_common::SymbolUrl getTemplateUrl();
        TemplateImport *getSuperTemplate();

        TemplateParameter getTemplateParameter(int index);
        std::vector<TemplateParameter>::const_iterator templateParametersBegin();
        std::vector<TemplateParameter>::const_iterator templateParametersEnd();
        int numTemplateParameters();

    private:
        tu_uint32 m_templateOffset;
        absl::Mutex m_lock;

        struct Priv;
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

         void load();
    };
}

#endif // LYRIC_IMPORTER_TEMPLATE_IMPORT_H
