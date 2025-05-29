
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_common/module_location.h>
#include <lyric_importer/module_cache.h>
#include <lyric_runtime/static_loader.h>
#include <lyric_typing/type_system.h>
#include <tempo_tracing/trace_recorder.h>

#include "base_typing_fixture.h"


void
BaseTypingFixture::SetUp()
{
    m_location = lyric_common::ModuleLocation::fromString("/test");
    auto staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    auto bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    auto localModuleCache = lyric_importer::ModuleCache::create(staticLoader);
    auto sharedModuleCache = lyric_importer::ModuleCache::create(bootstrapLoader);
    auto shortcutResolver = std::make_shared<lyric_importer::ShortcutResolver>();
    auto recorder = tempo_tracing::TraceRecorder::create();
    m_scopeManager = std::make_unique<tempo_tracing::ScopeManager>(recorder);
    auto span = m_scopeManager->makeSpan();
    span->setOperationName("BaseTypingFixture");

    m_objectState = std::make_unique<lyric_assembler::ObjectState>(
        m_location, localModuleCache, sharedModuleCache, shortcutResolver, m_scopeManager.get());
    TU_RAISE_IF_STATUS(m_objectState->defineRoot());

    m_typeSystem = std::make_unique<lyric_typing::TypeSystem>(m_objectState.get());
}
