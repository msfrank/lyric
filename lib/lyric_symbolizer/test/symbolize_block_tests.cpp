#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/object_state.h>
#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_importer/module_cache.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_runtime/static_loader.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_symbolizer/lyric_symbolizer.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_symbolizer_fixture.h"

class SymbolizeBlock : public BaseSymbolizerFixture {};

TEST_F(SymbolizeBlock, NoDefinitionsOrImports)
{
    auto symbolizeModuleResult = m_tester->symbolizeModule(R"(
        1 + 1
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (2, root.numSymbols());
    ASSERT_EQ (1, root.numImports());
}

TEST_F(SymbolizeBlock, DeclareImport)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.overrides = lyric_build::TaskSettings(tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
            {"sourceBaseUrl", tempo_config::ConfigValue("/src")},
        }},
    });
    lyric_test::LyricTester tester(testerOptions);
    ASSERT_TRUE (tester.configure().isOk());

    auto symbolizeModuleResult = tester.symbolizeModule(R"(
        import "/mod1" named mod1
        mod1.Foo()
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (2, root.numSymbols());
    ASSERT_EQ (2, root.numImports());

    auto import0 = root.getImport(0);
    auto import1 = root.getImport(1);
    absl::flat_hash_set<lyric_common::ModuleLocation> actual{
        import0.getImportLocation(),
        import1.getImportLocation()};

    absl::flat_hash_set<lyric_common::ModuleLocation> expected{
        lyric_common::ModuleLocation::fromString("/mod1"),
        lyric_bootstrap::preludeLocation()};

    ASSERT_EQ (expected, actual);
}
