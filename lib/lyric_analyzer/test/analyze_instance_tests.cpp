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

class AnalyzeInstance : public BaseAnalyzerFixture {};

TEST_F(AnalyzeInstance, DeclareInstance)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        definstance Foo {
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (4, object.numSymbols());
    ASSERT_EQ (1, object.numInstances());

    auto instance0 = object.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());
    ASSERT_EQ (lyric_object::AccessType::Public, instance0.getAccess());

    auto ctor = instance0.getInitializer();
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
}

TEST_F(AnalyzeInstance, DeclareInstanceWithExplicitInit)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        definstance Foo {
            var answer: Int
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
    ASSERT_EQ (1, object.numInstances());
    ASSERT_EQ (1, object.numFields());

    auto instance0 = object.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());

    auto IntType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")).orElseThrow();

    ASSERT_EQ (1, instance0.numMembers());
    auto field0 = instance0.getMember(0);
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (IntType, field0.getFieldType().getTypeDef());
    ASSERT_TRUE (field0.isVariable());
}


TEST_F(AnalyzeInstance, DeclareInstanceMemberVal)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        definstance Foo {
            val answer: Int = 42
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (5, object.numSymbols());
    ASSERT_EQ (1, object.numInstances());
    ASSERT_EQ (1, object.numFields());

    auto instance0 = object.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());

    auto IntType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")).orElseThrow();

    ASSERT_EQ (1, instance0.numMembers());
    auto field0 = instance0.getMember(0);
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (IntType, field0.getFieldType().getTypeDef());
    ASSERT_FALSE (field0.isVariable());
}

TEST_F(AnalyzeInstance, DeclareInstanceMemberVar)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        definstance Foo {
            var answer: Int = 42
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (5, object.numSymbols());
    ASSERT_EQ (1, object.numInstances());
    ASSERT_EQ (1, object.numFields());

    auto instance0 = object.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());

    auto IntType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")).orElseThrow();

    ASSERT_EQ (1, instance0.numMembers());
    auto field0 = instance0.getMember(0);
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (IntType, field0.getFieldType().getTypeDef());
    ASSERT_TRUE (field0.isVariable());
}

TEST_F(AnalyzeInstance, DeclareInstanceMethod)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        definstance Foo {
            def Identity(x: Int): Int { return x }
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (5, object.numSymbols());
    ASSERT_EQ (1, object.numInstances());
    ASSERT_EQ (3, object.numCalls());

    auto instance0 = object.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());
    ASSERT_EQ (2, instance0.numMethods());

    absl::flat_hash_map<std::string,lyric_object::CallWalker> instanceMethods;
    for (int i = 0; i < instance0.numMethods(); i++) {
        auto method = instance0.getMethod(i);
        instanceMethods[method.getSymbolPath().getName()] = method;
    }

    auto IntType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")).orElseThrow();

    ASSERT_TRUE (instanceMethods.contains("Identity"));
    auto identity = instanceMethods.at("Identity");
    ASSERT_TRUE (identity.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "Identity"}), identity.getSymbolPath());
    ASSERT_EQ (IntType, identity.getResultType().getTypeDef());

    ASSERT_EQ (1, identity.numListParameters());
    ASSERT_EQ (0, identity.numNamedParameters());
    auto param0 = identity.getListParameter(0);
    ASSERT_EQ ("x", param0.getParameterName());
    ASSERT_EQ (IntType, param0.getParameterType().getTypeDef());

    ASSERT_TRUE (instanceMethods.contains("$ctor"));
    auto ctor = instanceMethods.at("$ctor");
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
    ASSERT_TRUE (ctor.isConstructor());
}

TEST_F(AnalyzeInstance, DeclareInstanceImplMethod)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        definstance Foo {
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
    ASSERT_EQ (1, object.numInstances());
    ASSERT_EQ (3, object.numCalls());

    auto instance0 = object.getInstance(0);
    ASSERT_TRUE (instance0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), instance0.getSymbolPath());
    ASSERT_EQ (1, instance0.numImpls());

    auto BoolType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Bool")).orElseThrow();

    auto FooUrl = lyric_common::SymbolUrl::fromString("#Foo");
    auto FooType = lyric_common::TypeDef::forConcrete(FooUrl).orElseThrow();

    auto impl0 = instance0.getImpl(0);
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
