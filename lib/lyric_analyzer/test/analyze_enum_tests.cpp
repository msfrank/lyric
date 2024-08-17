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

TEST(AnalyzeEnum, DeclareEnum)
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
        defenum Foo {
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (4, root.numSymbols());
    ASSERT_EQ (1, root.numEnums());

    auto enum0 = root.getEnum(0);
    ASSERT_TRUE (enum0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), enum0.getSymbolPath());
    ASSERT_EQ (lyric_object::AccessType::Public, enum0.getAccess());

    auto ctor = enum0.getConstructor();
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
}

TEST(AnalyzeEnum, DeclareEnumCase)
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
        defenum Foo {
            case Bar
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (6, root.numSymbols());
    ASSERT_EQ (2, root.numEnums());

    auto enum0 = root.getEnum(0);
    ASSERT_TRUE (enum0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), enum0.getSymbolPath());

    auto ctor0 = enum0.getConstructor();
    ASSERT_TRUE (ctor0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor0.getSymbolPath());

    auto enum1 = root.getEnum(1);
    ASSERT_TRUE (enum1.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Bar"}), enum1.getSymbolPath());

    auto ctor1 = enum1.getConstructor();
    ASSERT_TRUE (ctor1.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Bar", "$ctor"}), ctor1.getSymbolPath());

    ASSERT_EQ (1, enum0.numSealedSubEnums());
    ASSERT_EQ (enum1.getEnumType().getTypeDef(), enum0.getSealedSubEnum(0).getTypeDef());
}

TEST(AnalyzeEnum, DeclareEnumMemberVal)
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
        defenum Foo {
            val answer: Int
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (5, root.numSymbols());
    ASSERT_EQ (1, root.numEnums());
    ASSERT_EQ (1, root.numFields());

    auto enum0 = root.getEnum(0);
    ASSERT_TRUE (enum0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), enum0.getSymbolPath());

    ASSERT_EQ (1, enum0.numMembers());
    auto field0 = enum0.getMember(0).getNearField();
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), field0.getFieldType().getTypeDef());
    ASSERT_FALSE (field0.isVariable());
}

TEST(AnalyzeEnum, DeclareEnumMethod)
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
        defenum Foo {
            def Identity(x: Int): Int { return x }
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (5, root.numSymbols());
    ASSERT_EQ (1, root.numEnums());
    ASSERT_EQ (3, root.numCalls());

    auto enum0 = root.getEnum(0);
    ASSERT_TRUE (enum0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), enum0.getSymbolPath());
    ASSERT_EQ (2, enum0.numMethods());

    absl::flat_hash_map<std::string,lyric_object::CallWalker> enumMethods;
    for (int i = 0; i < enum0.numMethods(); i++) {
        auto method = enum0.getMethod(i);
        auto call = method.getNearCall();
        enumMethods[call.getSymbolPath().getName()] = call;
    }

    ASSERT_TRUE (enumMethods.contains("Identity"));
    auto identity = enumMethods.at("Identity");
    ASSERT_TRUE (identity.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "Identity"}), identity.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), identity.getResultType().getTypeDef());

    ASSERT_EQ (1, identity.numListParameters());
    ASSERT_EQ (0, identity.numNamedParameters());
    auto param0 = identity.getListParameter(0);
    ASSERT_EQ ("x", param0.getParameterName());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), param0.getParameterType().getTypeDef());

    ASSERT_TRUE (enumMethods.contains("$ctor"));
    auto ctor = enumMethods.at("$ctor");
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
    ASSERT_TRUE (ctor.isConstructor());
}

TEST(AnalyzeEnum, DeclareEnumImplMethod)
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
        defenum Foo {
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
    ASSERT_EQ (1, root.numEnums());
    ASSERT_EQ (3, root.numCalls());

    auto enum0 = root.getEnum(0);
    ASSERT_TRUE (enum0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), enum0.getSymbolPath());
    ASSERT_EQ (1, enum0.numImpls());

    auto impl0 = enum0.getImpl(0);
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
