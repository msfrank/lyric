
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archiver_state.h>
#include <lyric_archiver/lyric_archiver.h>
#include <lyric_archiver/symbol_reference_set.h>

lyric_archiver::LyricArchiver::LyricArchiver(
    const lyric_common::ModuleLocation &location,
    const lyric_common::ModuleLocation &origin,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder,
    const ArchiverOptions &options)
    : m_location(location),
      m_origin(origin),
      m_localModuleCache(std::move(localModuleCache)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_shortcutResolver(std::move(shortcutResolver)),
      m_recorder(std::move(recorder)),
      m_options(options)
{
    TU_ASSERT (m_location.isValid());
    TU_ASSERT (m_origin.isValid());
}

tempo_utils::Status
lyric_archiver::LyricArchiver::initialize()
{
    if (m_archiverState != nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "archiver is already initialized");

    lyric_assembler::ObjectStateOptions objectStateOptions;

    auto objectState = std::make_unique<lyric_assembler::ObjectState>(m_location, m_origin,
        m_localModuleCache, m_systemModuleCache, m_shortcutResolver, objectStateOptions);

    lyric_assembler::ObjectRoot *objectRoot;
    TU_ASSIGN_OR_RETURN (objectRoot, objectState->defineRoot());

    auto archiverState = std::make_unique<ArchiverState>(
        std::move(objectState), m_systemModuleCache, objectRoot);

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

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_archiver::LyricArchiver::archiveSymbol(const lyric_common::SymbolUrl &symbolUrl)
{
    if (m_archiverState == nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "archiver is not initialized");

    SymbolReferenceSet symbolReferenceSet(m_archiverState.get());

    lyric_common::SymbolUrl archivedUrl;
    TU_ASSIGN_OR_RETURN (archivedUrl, m_archiverState->archiveSymbol(symbolUrl, symbolReferenceSet));

    while (!symbolReferenceSet.isEmpty()) {
        lyric_common::SymbolUrl referencedSymbolUrl;
        TU_ASSIGN_OR_RETURN (referencedSymbolUrl, symbolReferenceSet.takeReference());
        TU_RETURN_IF_STATUS (m_archiverState->archiveSymbol(referencedSymbolUrl, symbolReferenceSet));
    }

    TU_RETURN_IF_NOT_OK (m_archiverState->copyPendingProcs());

    return archivedUrl;
}

tempo_utils::Result<lyric_assembler::BindingSymbol *>
lyric_archiver::LyricArchiver::declareBinding(
    const std::string &name,
    bool isHidden,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    if (m_archiverState == nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "archiver is not initialized");

    return m_archiverState->declareBinding(name, isHidden, templateParameters);
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_archiver::LyricArchiver::toObject() const
{
    return m_archiverState->toObject();
}
