
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_common/module_location.h>
#include <lyric_importer/module_cache.h>
#include <lyric_typing/type_system.h>
#include <tempo_tracing/trace_recorder.h>

#include "base_fixture.h"

void
BaseFixture::SetUp()
{
    m_location = lyric_common::ModuleLocation::fromString("/test");
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    auto sharedModuleCache = lyric_importer::ModuleCache::create(loader);

    auto recorder = tempo_tracing::TraceRecorder::create();
    m_scopeManager = std::make_unique<tempo_tracing::ScopeManager>(recorder);
    auto span = m_scopeManager->makeSpan();
    span->setOperationName("BaseFixture");

    m_objectState = std::make_unique<lyric_assembler::ObjectState>(
        m_location, sharedModuleCache, m_scopeManager.get());
    TU_RAISE_IF_NOT_OK (m_objectState->initialize());

    m_typeSystem = std::make_unique<lyric_typing::TypeSystem>(m_objectState.get());
}