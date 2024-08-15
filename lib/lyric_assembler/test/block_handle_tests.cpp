#include <gtest/gtest.h>

//#include <lyric_packaging/package_loader.h>
//
//#include "assembly_state.h"
//#include "load_bootstrap.h"
//#include "module_entry.h"
//
//TEST(BlockHandle, TestBlockHandle)
//{
//    auto recorder = tempo_tracing::TraceRecorder::create();
//    tempo_tracing::ScopeManager scopeManager(recorder);
//    auto span = scopeManager.makeSpan();
//    span->setOperationName("TestBlockHandle");
//
//    // create fake state
//    lyric_packaging::PackageLoader loader({ZURI_DIST_LIB_PKG_DIR});
//    lyric_compiler::ObjectState state(
//        lyric_common::ModuleLocation("", "", "/test"),
//        lyric_common::ModuleLocation::fromString(ZURI_PRELUDE_LOCATION),
//        {},
//        &loader,
//        &scopeManager);
//
//    // load bootstrap symbols into the assembly state
//    auto status = lyric_compiler::load_bootstrap(state);
//    ASSERT_TRUE (status.isOk());
//
//    lyric_compiler::ModuleEntry moduleEntry(&state);
//    ASSERT_TRUE (moduleEntry.initialize(state.getLocation()).isOk());
//}