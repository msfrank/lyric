#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

TEST(AnalyzeClass, DeclareClass)
{
    lyric_test::TesterOptions testerOptions;
    testerOptions.buildConfig = tempo_config::ConfigMap{
        {"global", tempo_config::ConfigMap{
            {"preludeLocation", tempo_config::ConfigValue(BOOTSTRAP_PRELUDE_LOCATION)},
            {"bootstrapDirectoryPath", tempo_config::ConfigValue(LYRIC_BUILD_BOOTSTRAP_DIR)},
            {"sourceBaseUrl", tempo_config::ConfigValue("/src")},
        }},
    };
    lyric_test::LyricTester tester(testerOptions);
    ASSERT_TRUE (tester.configure().isOk());

    auto analyzeModuleResult = tester.analyzeModule(R"(
        defclass Foo {
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getAssembly();
    auto root = object.getObject();
    ASSERT_EQ (4, root.numSymbols());
    ASSERT_EQ (1, root.numClasses());

    auto class0 = root.getClass(0);
    ASSERT_TRUE (class0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), class0.getSymbolPath());

    auto ctor = class0.getConstructor();
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
}
