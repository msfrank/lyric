#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
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

TEST(AnalyzeClass, DeclareClassMemberVal)
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
            val answer: Int
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getAssembly();
    auto root = object.getObject();
    ASSERT_EQ (5, root.numSymbols());
    ASSERT_EQ (1, root.numClasses());
    ASSERT_EQ (1, root.numFields());

    auto class0 = root.getClass(0);
    ASSERT_TRUE (class0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), class0.getSymbolPath());

    ASSERT_EQ (1, class0.numMembers());
    auto field0 = class0.getMember(0).getNearField();
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), field0.getFieldType().getTypeDef());
    ASSERT_FALSE (field0.isVariable());
}

TEST(AnalyzeClass, DeclareClassMethod)
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
            def Identity(x: Int): Int { return x }
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getAssembly();
    auto root = object.getObject();
    ASSERT_EQ (5, root.numSymbols());
    ASSERT_EQ (1, root.numClasses());
    ASSERT_EQ (3, root.numCalls());

    auto class0 = root.getClass(0);
    ASSERT_TRUE (class0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), class0.getSymbolPath());
    ASSERT_EQ (2, class0.numMethods());

    absl::flat_hash_map<std::string,lyric_object::CallWalker> classMethods;
    for (int i = 0; i < class0.numMethods(); i++) {
        auto method = class0.getMethod(i);
        auto call = method.getNearCall();
        classMethods[call.getSymbolPath().getName()] = call;
    }

    ASSERT_TRUE (classMethods.contains("Identity"));
    auto identity = classMethods.at("Identity");
    ASSERT_TRUE (identity.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "Identity"}), identity.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), identity.getResultType().getTypeDef());

    ASSERT_EQ (1, identity.numListParameters());
    ASSERT_EQ (0, identity.numNamedParameters());
    auto param0 = identity.getListParameter(0);
    ASSERT_EQ ("x", param0.getParameterName());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), param0.getParameterType().getTypeDef());

    ASSERT_TRUE (classMethods.contains("$ctor"));
    auto ctor = classMethods.at("$ctor");
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
    ASSERT_TRUE (ctor.isConstructor());
}
