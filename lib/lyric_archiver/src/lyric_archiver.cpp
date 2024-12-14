
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archiver_state.h>
#include <lyric_archiver/lyric_archiver.h>

lyric_archiver::LyricArchiver::LyricArchiver(
    const lyric_common::ModuleLocation &location,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder,
    const ArchiverOptions &options)
    : m_location(location),
      m_localModuleCache(std::move(localModuleCache)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_recorder(recorder),
      m_options(options)
{
}

tempo_utils::Status
lyric_archiver::LyricArchiver::initialize()
{
    if (m_archiverState != nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "archiver is already initialized");

    auto scopeManager = std::make_unique<tempo_tracing::ScopeManager>(m_recorder);

    lyric_assembler::ObjectStateOptions objectStateOptions;

    auto objectState = std::make_unique<lyric_assembler::ObjectState>(
        m_location, m_localModuleCache, m_systemModuleCache, scopeManager.get(), objectStateOptions);

    lyric_assembler::ObjectRoot *objectRoot;
    TU_ASSIGN_OR_RETURN (objectRoot, objectState->defineRoot());

    auto archiverState = std::make_unique<ArchiverState>(
        std::move(objectState), m_systemModuleCache, objectRoot);

    m_scopeManager = std::move(scopeManager);
    m_archiverState = std::move(archiverState);

    return {};
}

tempo_utils::Status
lyric_archiver::LyricArchiver::importModule(const lyric_common::ModuleLocation &location)
{
    if (m_archiverState == nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "archiver is not initialized");

    return m_archiverState->importObject(location);
}

tempo_utils::Status
lyric_archiver::LyricArchiver::insertModule(
    const lyric_common::ModuleLocation &location,
    const lyric_object::LyricObject &object)
{
    if (m_archiverState == nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "archiver is not initialized");

    return m_archiverState->insertObject(location, object);
}

tempo_utils::Status
lyric_archiver::LyricArchiver::archiveSymbol(
    const lyric_common::ModuleLocation &srcLocation,
    const std::string &srcIdentifier,
    const std::string &dstIdentifier,
    lyric_object::AccessType access)
{
    lyric_common::SymbolUrl symbolUrl(srcLocation, lyric_common::SymbolPath({srcIdentifier}));
    return archiveSymbol(symbolUrl, dstIdentifier, access);
}

tempo_utils::Status
lyric_archiver::LyricArchiver::archiveSymbol(
    const lyric_common::SymbolUrl &symbolUrl,
    const std::string &identifier,
    lyric_object::AccessType access)
{
    if (m_archiverState == nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "archiver is not initialized");

    TU_RETURN_IF_NOT_OK (m_archiverState->archiveSymbol(symbolUrl, identifier, access));
    TU_RETURN_IF_NOT_OK (m_archiverState->performFixups());

    return {};
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_archiver::LyricArchiver::toObject() const
{
    return m_archiverState->toObject();
}
