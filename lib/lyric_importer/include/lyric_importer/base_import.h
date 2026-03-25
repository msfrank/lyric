#ifndef LYRIC_IMPORTER_BASE_IMPORT_H
#define LYRIC_IMPORTER_BASE_IMPORT_H

#include "module_import.h"

namespace lyric_importer {

    class BaseImport {
    public:
        explicit BaseImport(std::weak_ptr<ModuleImport> moduleImport);

        std::weak_ptr<ModuleImport> getModuleImport() const;
        std::shared_ptr<ModuleImport> acquireModuleImport() const;
        void releaseModuleImport();

    private:
        std::weak_ptr<ModuleImport> m_moduleImport;
    };
}

#endif // LYRIC_IMPORTER_BASE_IMPORT_H
