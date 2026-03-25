
#include <lyric_importer/base_import.h>

lyric_importer::BaseImport::BaseImport(std::weak_ptr<ModuleImport> moduleImport)
    : m_moduleImport(std::move(moduleImport))
{
    TU_ASSERT (!m_moduleImport.expired());
}

std::weak_ptr<lyric_importer::ModuleImport>
lyric_importer::BaseImport::getModuleImport() const
{
    return m_moduleImport;
}

std::shared_ptr<lyric_importer::ModuleImport>
lyric_importer::BaseImport::acquireModuleImport() const
{
    return m_moduleImport.lock();
}

void
lyric_importer::BaseImport::releaseModuleImport()
{
    m_moduleImport.reset();
}