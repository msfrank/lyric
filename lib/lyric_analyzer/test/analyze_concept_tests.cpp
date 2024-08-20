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

TEST(AnalyzeConcept, DeclareConcept)
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
        defconcept Foo {
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (1, root.numConcepts());

    auto concept0 = root.getConcept(0);
    ASSERT_TRUE (concept0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), concept0.getSymbolPath());
    ASSERT_EQ (lyric_object::AccessType::Public, concept0.getAccess());
}

TEST(AnalyzeConcept, DeclareConceptAction)
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
        defconcept Foo {
            decl Identity(x: Int): Int
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (4, root.numSymbols());
    ASSERT_EQ (1, root.numConcepts());
    ASSERT_EQ (1, root.numActions());

    auto concept0 = root.getConcept(0);
    ASSERT_TRUE (concept0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), concept0.getSymbolPath());

    ASSERT_EQ (1, concept0.numActions());
    auto action0 = concept0.getAction(0).getNearAction();
    ASSERT_TRUE (action0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "Identity"}), action0.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), action0.getResultType().getTypeDef());

    ASSERT_EQ (1, action0.numListParameters());
    ASSERT_EQ (0, action0.numNamedParameters());
    auto param0 = action0.getListParameter(0);
    ASSERT_EQ ("x", param0.getParameterName());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), param0.getParameterType().getTypeDef());
}

TEST(AnalyzeConcept, DeclareConceptImplMethod)
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
        defconcept Foo {
            impl Equality[Foo,Foo] {
                def equals(lhs: Foo, rhs: Foo): Bool { false }
            }
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (4, root.numSymbols());
    ASSERT_EQ (1, root.numConcepts());
    ASSERT_EQ (2, root.numCalls());

    auto concept0 = root.getConcept(0);
    ASSERT_TRUE (concept0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), concept0.getSymbolPath());
    ASSERT_EQ (1, concept0.numImpls());

    auto impl0 = concept0.getImpl(0);
    ASSERT_TRUE (impl0.isValid());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(
        lyric_bootstrap::preludeSymbol("Equality"), {
            lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Foo")),
            lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Foo")),
        }),
               impl0.getImplType().getTypeDef());

    auto extension0 = impl0.getExtension(0);
    ASSERT_TRUE (extension0.isValid());
    ASSERT_EQ (lyric_object::AddressType::Far, extension0.actionAddressType());
    auto extensionAction = extension0.getFarAction();
    ASSERT_EQ (lyric_bootstrap::preludeSymbol("Equality.equals"), extensionAction.getLinkUrl());

    auto equals = extension0.getNearCall();
    ASSERT_TRUE (equals.isDeclOnly());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Bool")), equals.getResultType().getTypeDef());

    ASSERT_EQ (2, equals.numListParameters());
    ASSERT_EQ (0, equals.numNamedParameters());
    auto param0 = equals.getListParameter(0);
    ASSERT_EQ ("lhs", param0.getParameterName());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Foo")), param0.getParameterType().getTypeDef());
    auto param1 = equals.getListParameter(1);
    ASSERT_EQ ("rhs", param1.getParameterName());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl::fromString("#Foo")), param1.getParameterType().getTypeDef());
}
