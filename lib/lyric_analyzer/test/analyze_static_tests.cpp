#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

inline lyric_common::AssemblyLocation
preludeLocation()
{
    return lyric_common::AssemblyLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION);
}

inline lyric_common::SymbolUrl
preludeSymbol(std::string_view symbolName)
{
    return lyric_common::SymbolUrl(preludeLocation(), lyric_common::SymbolPath({std::string(symbolName)}));
}

TEST(AnalyzeStatic, DeclareStaticVal)
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
        val Static: Int = 0
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getAssembly();
    auto root = object.getObject();
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (1, root.numStatics());

    auto static1 = root.getStatic(0);
    ASSERT_EQ (lyric_common::SymbolPath({"Static"}), static1.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(preludeSymbol("Int")), static1.getStaticType().getTypeDef());
    ASSERT_FALSE (static1.isVariable());
}

TEST(AnalyzeStatic, DeclareStaticVar)
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
        var Static: Int = 0
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getAssembly();
    auto root = object.getObject();
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (1, root.numStatics());

    auto static1 = root.getStatic(0);
    ASSERT_EQ (lyric_common::SymbolPath({"Static"}), static1.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(preludeSymbol("Int")), static1.getStaticType().getTypeDef());
    ASSERT_TRUE (static1.isVariable());
}
