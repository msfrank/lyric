#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_object/extension_walker.h>
#include <lyric_object/parameter_walker.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/common_printers.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_analyzer_fixture.h"

class AnalyzeEnum : public BaseAnalyzerFixture {};

TEST_F(AnalyzeEnum, DeclareEnum)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defenum Foo {
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (4, object.numSymbols());
    ASSERT_EQ (1, object.numEnums());

    auto enum0 = object.getEnum(0);
    ASSERT_TRUE (enum0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), enum0.getSymbolPath());
    ASSERT_EQ (lyric_object::AccessType::Public, enum0.getAccess());

    auto ctor = enum0.getInitializer();
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
}

TEST_F(AnalyzeEnum, DeclareEnumWithExplicitInit)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defenum Foo {
            val answer: Int
            init() {
                set this.answer = 42
            }
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (5, object.numSymbols());
    ASSERT_EQ (1, object.numEnums());
    ASSERT_EQ (1, object.numFields());

    auto enum0 = object.getEnum(0);
    ASSERT_TRUE (enum0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), enum0.getSymbolPath());

    auto IntType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")).orElseThrow();

    ASSERT_EQ (1, enum0.numMembers());
    auto field0 = enum0.getMember(0);
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (IntType, field0.getFieldType().getTypeDef());
    ASSERT_FALSE (field0.isVariable());
}

TEST_F(AnalyzeEnum, DeclareEnumCase)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defenum Foo {
            case Bar
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (6, object.numSymbols());
    ASSERT_EQ (2, object.numEnums());

    auto FooEnum = object.getEnum(object.findSymbol(lyric_common::SymbolPath({"Foo"})).getLinkageIndex());
    ASSERT_TRUE (FooEnum.isValid());
    ASSERT_TRUE (FooEnum.isDeclOnly());

    auto ctor0 = FooEnum.getInitializer();
    ASSERT_TRUE (ctor0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor0.getSymbolPath());

    auto BarEnum = object.getEnum(object.findSymbol(lyric_common::SymbolPath({"Bar"})).getLinkageIndex());
    ASSERT_TRUE (BarEnum.isValid());
    ASSERT_TRUE (BarEnum.isDeclOnly());

    auto ctor1 = BarEnum.getInitializer();
    ASSERT_TRUE (ctor1.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Bar", "$ctor"}), ctor1.getSymbolPath());

    ASSERT_EQ (1, FooEnum.numSealedSubEnums());
    ASSERT_EQ (BarEnum.getEnumType().getTypeDef(), FooEnum.getSealedSubEnum(0).getTypeDef());
}

TEST_F(AnalyzeEnum, DeclareEnumMemberVal)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defenum Foo {
            val answer: Int
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (5, object.numSymbols());
    ASSERT_EQ (1, object.numEnums());
    ASSERT_EQ (1, object.numFields());

    auto enum0 = object.getEnum(0);
    ASSERT_TRUE (enum0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), enum0.getSymbolPath());

    auto IntType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")).orElseThrow();

    ASSERT_EQ (1, enum0.numMembers());
    auto field0 = enum0.getMember(0);
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (IntType, field0.getFieldType().getTypeDef());
    ASSERT_FALSE (field0.isVariable());
}

TEST_F(AnalyzeEnum, DeclareEnumMethod)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defenum Foo {
            def Identity(x: Int): Int { return x }
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (5, object.numSymbols());
    ASSERT_EQ (1, object.numEnums());
    ASSERT_EQ (3, object.numCalls());

    auto enum0 = object.getEnum(0);
    ASSERT_TRUE (enum0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), enum0.getSymbolPath());
    ASSERT_EQ (2, enum0.numMethods());

    absl::flat_hash_map<std::string,lyric_object::CallWalker> enumMethods;
    for (int i = 0; i < enum0.numMethods(); i++) {
        auto method = enum0.getMethod(i);
        enumMethods[method.getSymbolPath().getName()] = method;
    }

    auto IntType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")).orElseThrow();

    ASSERT_TRUE (enumMethods.contains("Identity"));
    auto identity = enumMethods.at("Identity");
    ASSERT_TRUE (identity.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "Identity"}), identity.getSymbolPath());
    ASSERT_EQ (IntType, identity.getResultType().getTypeDef());

    ASSERT_EQ (1, identity.numListParameters());
    ASSERT_EQ (0, identity.numNamedParameters());
    auto param0 = identity.getListParameter(0);
    ASSERT_EQ ("x", param0.getParameterName());
    ASSERT_EQ (IntType, param0.getParameterType().getTypeDef());

    ASSERT_TRUE (enumMethods.contains("$ctor"));
    auto ctor = enumMethods.at("$ctor");
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
    ASSERT_TRUE (ctor.isConstructor());
}

TEST_F(AnalyzeEnum, DeclareEnumImplMethod)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defenum Foo {
            impl Equality[Foo,Foo] {
                def Equals(lhs: Foo, rhs: Foo): Bool { false }
            }
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (5, object.numSymbols());
    ASSERT_EQ (1, object.numEnums());
    ASSERT_EQ (3, object.numCalls());

    auto enum0 = object.getEnum(0);
    ASSERT_TRUE (enum0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), enum0.getSymbolPath());
    ASSERT_EQ (1, enum0.numImpls());

    auto BoolType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Bool")).orElseThrow();

    auto FooUrl = lyric_common::SymbolUrl::fromString("#Foo");
    auto FooType = lyric_common::TypeDef::forConcrete(FooUrl).orElseThrow();

    auto impl0 = enum0.getImpl(0);
    ASSERT_TRUE (impl0.isValid());
    auto EqualityOfFooFooType = lyric_common::TypeDef::forConcrete(
        lyric_bootstrap::preludeSymbol("Equality"), {FooType, FooType})
        .orElseThrow();
    ASSERT_EQ (EqualityOfFooFooType, impl0.getImplType().getTypeDef());

    auto extension0 = impl0.getExtension(0);
    ASSERT_TRUE (extension0.isValid());
    ASSERT_EQ (lyric_object::AddressType::Far, extension0.actionAddressType());
    auto extensionAction = extension0.getFarAction();
    ASSERT_EQ (lyric_bootstrap::preludeSymbol("Equality.Equals"), extensionAction.getLinkUrl());

    auto equals = extension0.getNearCall();
    ASSERT_TRUE (equals.isDeclOnly());
    ASSERT_EQ (BoolType, equals.getResultType().getTypeDef());

    ASSERT_EQ (2, equals.numListParameters());
    ASSERT_EQ (0, equals.numNamedParameters());
    auto param0 = equals.getListParameter(0);
    ASSERT_EQ ("lhs", param0.getParameterName());
    ASSERT_EQ (FooType, param0.getParameterType().getTypeDef());
    auto param1 = equals.getListParameter(1);
    ASSERT_EQ ("rhs", param1.getParameterName());
    ASSERT_EQ (FooType, param1.getParameterType().getTypeDef());
}
