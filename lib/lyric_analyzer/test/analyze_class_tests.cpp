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

class AnalyzeClass : public BaseAnalyzerFixture {};

TEST_F(AnalyzeClass, DeclareClass)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defclass Foo {
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    TU_LOG_INFO << object.dumpJson();
    ASSERT_EQ (4, object.numSymbols());
    ASSERT_EQ (1, object.numClasses());

    auto class0 = object.getClass(0);
    ASSERT_TRUE (class0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), class0.getSymbolPath());
    ASSERT_EQ (lyric_object::AccessType::Public, class0.getAccess());

    auto ctor = class0.getConstructor();
    ASSERT_TRUE (ctor.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "$ctor"}), ctor.getSymbolPath());
}

TEST_F(AnalyzeClass, DeclareClassMemberVal)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defclass Foo {
            val answer: Int
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (5, object.numSymbols());
    ASSERT_EQ (1, object.numClasses());
    ASSERT_EQ (1, object.numFields());

    auto class0 = object.getClass(0);
    ASSERT_TRUE (class0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), class0.getSymbolPath());

    ASSERT_EQ (1, class0.numMembers());
    auto field0 = class0.getMember(0);
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), field0.getFieldType().getTypeDef());
    ASSERT_FALSE (field0.isVariable());
}

TEST_F(AnalyzeClass, DeclareClassMemberVar)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defclass Foo {
            var answer: Int
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (5, object.numSymbols());
    ASSERT_EQ (1, object.numClasses());
    ASSERT_EQ (1, object.numFields());

    auto class0 = object.getClass(0);
    ASSERT_TRUE (class0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), class0.getSymbolPath());

    ASSERT_EQ (1, class0.numMembers());
    auto field0 = class0.getMember(0);
    ASSERT_TRUE (field0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "answer"}), field0.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), field0.getFieldType().getTypeDef());
    ASSERT_TRUE (field0.isVariable());
}

TEST_F(AnalyzeClass, DeclareClassMethod)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defclass Foo {
            def Identity(x: Int): Int { return x }
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (5, object.numSymbols());
    ASSERT_EQ (1, object.numClasses());
    ASSERT_EQ (3, object.numCalls());

    auto class0 = object.getClass(0);
    ASSERT_TRUE (class0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), class0.getSymbolPath());
    ASSERT_EQ (2, class0.numMethods());

    absl::flat_hash_map<std::string,lyric_object::CallWalker> classMethods;
    for (int i = 0; i < class0.numMethods(); i++) {
        auto method = class0.getMethod(i);
        classMethods[method.getSymbolPath().getName()] = method;
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

TEST_F(AnalyzeClass, DeclareClassImplMethod)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defclass Foo {
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
    ASSERT_EQ (1, object.numClasses());
    ASSERT_EQ (3, object.numCalls());

    auto class0 = object.getClass(0);
    ASSERT_TRUE (class0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), class0.getSymbolPath());
    ASSERT_EQ (1, class0.numImpls());

    auto impl0 = class0.getImpl(0);
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
    ASSERT_EQ (lyric_bootstrap::preludeSymbol("Equality.Equals"), extensionAction.getLinkUrl());

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
