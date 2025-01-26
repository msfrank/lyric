#ifndef LYRIC_IMPORTER_BASE_IMPORT_H
#define LYRIC_IMPORTER_BASE_IMPORT_H

#include "module_import.h"

namespace lyric_importer {

    class BaseImport {
    public:
        explicit BaseImport(std::shared_ptr<ModuleImport> moduleImport);

        std::shared_ptr<ModuleImport> getModuleImport() const;

    private:
        std::shared_ptr<ModuleImport> m_moduleImport;
    };
}

#endif // LYRIC_IMPORTER_BASE_IMPORT_H
