#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/common_printers.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

TEST(AnalyzeInstance, DeclareInstance)
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
        definstance Foo {
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (4, root.numSymbols());
    ASSERT_EQ (1, root.numInstances());

    auto instance0 = root.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());
    ASSERT_EQ (lyric_object::AccessType::Public, instance0.getAccess());

    auto ctor = instance0.getConstructor();
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
}

TEST(AnalyzeInstance, DeclareInstanceMemberVal)
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
        definstance Foo {
            val answer: Int
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (5, root.numSymbols());
    ASSERT_EQ (1, root.numInstances());
    ASSERT_EQ (1, root.numFields());

    auto instance0 = root.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());

    ASSERT_EQ (1, instance0.numMembers());
    auto field0 = instance0.getMember(0).getNearField();
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), field0.getFieldType().getTypeDef());
    ASSERT_FALSE (field0.isVariable());
}

TEST(AnalyzeInstance, DeclareInstanceMemberVar)
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
        definstance Foo {
            var answer: Int
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (5, root.numSymbols());
    ASSERT_EQ (1, root.numInstances());
    ASSERT_EQ (1, root.numFields());

    auto instance0 = root.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());

    ASSERT_EQ (1, instance0.numMembers());
    auto field0 = instance0.getMember(0).getNearField();
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), field0.getFieldType().getTypeDef());
    ASSERT_TRUE (field0.isVariable());
}

TEST(AnalyzeInstance, DeclareInstanceMethod)
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
        definstance Foo {
            def Identity(x: Int): Int { return x }
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (5, root.numSymbols());
    ASSERT_EQ (1, root.numInstances());
    ASSERT_EQ (3, root.numCalls());

    auto instance0 = root.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());
    ASSERT_EQ (2, instance0.numMethods());

    absl::flat_hash_map<std::string,lyric_object::CallWalker> instanceMethods;
    for (int i = 0; i < instance0.numMethods(); i++) {
        auto method = instance0.getMethod(i);
        auto call = method.getNearCall();
        instanceMethods[call.getSymbolPath().getName()] = call;
    }

    ASSERT_TRUE (instanceMethods.contains("Identity"));
    auto identity = instanceMethods.at("Identity");
    ASSERT_TRUE (identity.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "Identity"}), identity.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), identity.getResultType().getTypeDef());

    ASSERT_EQ (1, identity.numListParameters());
    ASSERT_EQ (0, identity.numNamedParameters());
    auto param0 = identity.getListParameter(0);
    ASSERT_EQ ("x", param0.getParameterName());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), param0.getParameterType().getTypeDef());

    ASSERT_TRUE (instanceMethods.contains("$ctor"));
    auto ctor = instanceMethods.at("$ctor");
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
    ASSERT_TRUE (ctor.isConstructor());
}

TEST(AnalyzeInstance, DeclareInstanceImplMethod)
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
        definstance Foo {
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
    ASSERT_EQ (5, root.numSymbols());
    ASSERT_EQ (1, root.numInstances());
    ASSERT_EQ (3, root.numCalls());

    auto instance0 = root.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());
    ASSERT_EQ (1, instance0.numImpls());

    auto impl0 = instance0.getImpl(0);
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
