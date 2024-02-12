#include <gtest/gtest.h>

#include <lyric_typing/callsite_reifier.h>
#include "lyric_bootstrap/bootstrap_loader.h"

TEST(CallsiteReifier, ReifierWithNoTypeParametersIsValid)
{
    auto recorder = tempo_tracing::TraceRecorder::create();
    tempo_tracing::ScopeManager scopeManager(recorder);
    auto span = scopeManager.makeSpan();
    span->setOperationName("test");

    auto location = lyric_common::AssemblyLocation::fromString("/test");
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    auto sharedModuleCache = lyric_importer::ModuleCache::create(loader);
    lyric_assembler::AssemblyState assemblyState(location, sharedModuleCache, &scopeManager);
    TU_RAISE_IF_NOT_OK (assemblyState.initialize());

    lyric_typing::TypeSystem typeSystem(&assemblyState);

    lyric_typing::CallsiteReifier reifier({}, {}, &typeSystem);
    ASSERT_TRUE (reifier.isValid());
    ASSERT_EQ (0, reifier.numArguments());
}
