
#include <lyric_importer/base_import.h>

lyric_importer::BaseImport::BaseImport(std::shared_ptr<ModuleImport> moduleImport)
    : m_moduleImport(moduleImport)
{
    TU_ASSERT (m_moduleImport != nullptr);
}

std::shared_ptr<lyric_importer::ModuleImport>
lyric_importer::BaseImport::getModuleImport() const
{
    return m_moduleImport;
}