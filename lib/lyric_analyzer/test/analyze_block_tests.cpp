#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_analyzer/lyric_analyzer.h>
#include <lyric_assembler/object_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_importer/module_cache.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_runtime/static_loader.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <tempo_test/status_matchers.h>

TEST(AnalyzeBlock, NoDefinitions)
{
    lyric_parser::LyricParser parser({});

    auto sourceUrl = tempo_utils::Url::fromString("/test");
    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        1 + 1
    )", sourceUrl, recorder);

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    auto bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    auto localModuleCache = lyric_importer::ModuleCache::create(staticLoader);
    auto systemModuleCache = lyric_importer::ModuleCache::create(bootstrapLoader);
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("tester://", tempo_utils::UUID::randomUUID().toString()));

    lyric_analyzer::AnalyzerOptions options;
    lyric_analyzer::LyricAnalyzer analyzer(origin, localModuleCache, systemModuleCache, options);

    lyric_assembler::ObjectStateOptions objectStateOptions;
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RAISE (object, analyzer.analyzeModule(location, archetype, objectStateOptions, recorder));

    ASSERT_EQ (2, object.numSymbols());
}