
#include <lyric_assembler/block_handle.h>
#include <lyric_runtime/static_loader.h>

#include "base_optimizer_fixture.h"

BaseOptimizerFixture::BaseOptimizerFixture()
{
    m_staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    m_testerOptions.fallbackLoader = m_staticLoader;
    m_testerOptions.buildConfig = tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
        }},
    };
}

tempo_utils::Status
BaseOptimizerFixture::configure()
{
    m_tester = std::make_unique<lyric_test::LyricTester>(m_testerOptions);
    TU_RETURN_IF_NOT_OK (m_tester->configure());

    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    auto localModuleCache = lyric_importer::ModuleCache::create(staticLoader);
    auto sharedModuleCache = m_tester->getRunner()->getBuilder()->getSharedModuleCache();
    auto recorder = tempo_tracing::TraceRecorder::create();
    tempo_tracing::ScopeManager scopeManager(recorder);
    m_objectState = std::make_unique<lyric_assembler::ObjectState>(
        location, localModuleCache, sharedModuleCache, &scopeManager);

    TU_ASSIGN_OR_RAISE (m_objectRoot, m_objectState->defineRoot());

    return {};
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
BaseOptimizerFixture::declareFunction(
    const std::string &name,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    auto *rootBlock = m_objectRoot->rootBlock();
    return rootBlock->declareFunction(name, access, templateParameters);
}
