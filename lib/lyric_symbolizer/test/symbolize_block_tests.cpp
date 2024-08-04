#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/assembly_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_importer/module_cache.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_symbolizer/lyric_symbolizer.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_test/result_matchers.h>

TEST(SymbolizeBlock, NoDefinitionsOrImports)
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

    lyric_symbolizer::SymbolizerOptions options;
    lyric_symbolizer::LyricSymbolizer symbolizer(systemModuleCache, options);

    lyric_assembler::AssemblyStateOptions assemblyStateOptions;
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RAISE (object, symbolizer.symbolizeModule(location, archetype, assemblyStateOptions, recorder));

    auto root = object.getObject();
    ASSERT_EQ (0, root.numSymbols());
    ASSERT_EQ (0, root.numImports());
}

TEST(SymbolizeBlock, DeclareImport)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.buildConfig = tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
            {"sourceBaseUrl", tempo_config::ConfigValue("/src")},
        }},
    };
    lyric_test::LyricTester tester(testerOptions);
    ASSERT_TRUE (tester.configure().isOk());

    auto compileModuleResult = tester.compileModule("1 + 1", "mod1");
    ASSERT_THAT (compileModuleResult,
        tempo_test::ContainsResult(CompileModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModuleResult = tester.symbolizeModule(R"(
        import "/mod1" named mod1
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getAssembly();
    auto root = object.getObject();
    ASSERT_EQ (1, root.numImports());

    auto import1 = root.getImport(0);
    ASSERT_EQ (lyric_common::AssemblyLocation::fromString("/mod1"), import1.getImportLocation());
}
