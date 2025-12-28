
#include <lyric_runtime/static_loader.h>

#include "base_archiver_fixture.h"

BaseArchiverFixture::BaseArchiverFixture()
{
    m_staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    m_testerOptions.fallbackLoader = m_staticLoader;
    m_testerOptions.bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
}

tempo_utils::Status
BaseArchiverFixture::configure(std::string_view archiveUrlString)
{
    m_archiveLocation = lyric_common::ModuleLocation::fromString(archiveUrlString);
    TU_ASSERT (m_archiveLocation.isAbsolute());
    m_tester = std::make_unique<lyric_test::LyricTester>(m_testerOptions);
    TU_RETURN_IF_NOT_OK (m_tester->configure());
    m_originLocation = lyric_common::ModuleLocation::fromString(
        absl::StrCat("tester://", tempo_utils::UUID::randomUUID().toString()));
    TU_ASSERT (m_originLocation.isValid());
    m_archiverTester = std::make_unique<ArchiverTester>(m_originLocation, m_tester.get());
    return {};
}

tempo_utils::Result<lyric_common::ModuleLocation>
BaseArchiverFixture::writeModule(
    const std::string &code,
    const std::filesystem::path &path)
{
    return m_archiverTester->writeModule(code, path);
}

tempo_utils::Status
BaseArchiverFixture::prepare()
{
    auto *builder = m_tester->getRunner()->getBuilder();
    auto sharedModuleCache = builder->getSharedModuleCache();
    auto shortcutResolver = builder->getShortcutResolver();

    std::shared_ptr<lyric_runtime::AbstractLoader> archiverLoader;
    TU_ASSIGN_OR_RETURN (archiverLoader, m_archiverTester->build());
    auto localModuleCache = lyric_importer::ModuleCache::create(archiverLoader);

    lyric_archiver::ArchiverOptions options;
    options.preludeLocation = lyric_common::ModuleLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION);

    auto recorder = tempo_tracing::TraceRecorder::create();
    auto archiveLocation = lyric_common::ModuleLocation::fromUrlPath(m_archiveLocation.getPath());
    m_archiver = std::make_unique<lyric_archiver::LyricArchiver>(
        archiveLocation, m_originLocation, localModuleCache, sharedModuleCache,
        shortcutResolver, recorder, options);

    return m_archiver->initialize();
}

tempo_utils::Status
BaseArchiverFixture::importModule(const lyric_common::ModuleLocation &location)
{
    return m_archiver->importModule(location);
}

tempo_utils::Result<lyric_common::SymbolUrl>
BaseArchiverFixture::archiveSymbol(
    const lyric_common::ModuleLocation &location,
    const std::string &identifier)
{
    lyric_common::SymbolUrl symbolUrl(location, lyric_common::SymbolPath({identifier}));
    return m_archiver->archiveSymbol(symbolUrl);
}

tempo_utils::Result<lyric_assembler::BindingSymbol *>
BaseArchiverFixture::declareBinding(
    const std::string &name,
    bool isHidden,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    return m_archiver->declareBinding(name, isHidden, templateParameters);
}

tempo_utils::Status
BaseArchiverFixture::build()
{
    lyric_object::LyricObject archiveObject;
    TU_ASSIGN_OR_RAISE (archiveObject, m_archiver->toObject());
    return m_staticLoader->insertModule(m_archiveLocation, archiveObject);
}

tempo_utils::Result<lyric_test::RunModule>
BaseArchiverFixture::runCode(const std::string &code)
{
    return m_tester->runModule(code);
}