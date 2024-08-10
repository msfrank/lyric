#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_analyzer/lyric_analyzer.h>
#include <lyric_assembler/assembly_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_importer/module_cache.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <tempo_test/status_matchers.h>

TEST(AnalyzeBlock, NoDefinitions)
{
    lyric_parser::LyricParser parser({});
    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        1 + 1
    )", {}, recorder);

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    auto location = lyric_common::AssemblyLocation::fromString("/test");
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    auto systemModuleCache = lyric_importer::ModuleCache::create(loader);

    lyric_analyzer::AnalyzerOptions options;
    lyric_analyzer::LyricAnalyzer analyzer(systemModuleCache, options);

    lyric_assembler::AssemblyStateOptions assemblyStateOptions;
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RAISE (object, analyzer.analyzeModule(location, archetype, assemblyStateOptions, recorder));

    auto root = object.getObject();
    ASSERT_EQ (2, root.numSymbols());
}