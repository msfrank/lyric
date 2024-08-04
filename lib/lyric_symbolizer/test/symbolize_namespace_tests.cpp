#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

TEST(SymbolizeNamespace, DeclareNamespace)
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

    auto symbolizeModuleResult = tester.symbolizeModule(R"(
        namespace Namespace {
            1
        }
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getAssembly();
    auto root = object.getObject();
    ASSERT_EQ (0, root.numImports());
    ASSERT_EQ (1, root.numSymbols());

    auto symbol1 = root.getSymbol(0);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Namespace"), symbol1.getSymbolPath());
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Namespace);
    ASSERT_EQ (symbol1.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);
}
