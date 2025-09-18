#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_object/extension_walker.h>
#include <lyric_object/parameter_walker.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_analyzer_fixture.h"

class AnalyzeConcept : public BaseAnalyzerFixture {};

TEST_F(AnalyzeConcept, DeclareConcept)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defconcept Foo {
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (3, object.numSymbols());
    ASSERT_EQ (1, object.numConcepts());

    auto concept0 = object.getConcept(0);
    ASSERT_TRUE (concept0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), concept0.getSymbolPath());
    ASSERT_EQ (lyric_object::AccessType::Public, concept0.getAccess());
}

TEST_F(AnalyzeConcept, DeclareConceptAction)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defconcept Foo {
            decl Identity(x: Int): Int
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (4, object.numSymbols());
    ASSERT_EQ (1, object.numConcepts());
    ASSERT_EQ (1, object.numActions());

    auto concept0 = object.getConcept(0);
    ASSERT_TRUE (concept0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), concept0.getSymbolPath());

    auto IntType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")).orElseThrow();

    ASSERT_EQ (1, concept0.numActions());
    auto action0 = concept0.getAction(0).getNearAction();
    ASSERT_TRUE (action0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo", "Identity"}), action0.getSymbolPath());
    ASSERT_EQ (IntType, action0.getResultType().getTypeDef());

    ASSERT_EQ (1, action0.numListParameters());
    ASSERT_EQ (0, action0.numNamedParameters());
    auto param0 = action0.getListParameter(0);
    ASSERT_EQ ("x", param0.getParameterName());
    ASSERT_EQ (IntType, param0.getParameterType().getTypeDef());
}

TEST_F(AnalyzeConcept, DeclareConceptImplMethod)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        defconcept Foo {
            impl Equality[Foo,Foo] {
                def Equals(lhs: Foo, rhs: Foo): Bool { false }
            }
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (4, object.numSymbols());
    ASSERT_EQ (1, object.numConcepts());
    ASSERT_EQ (2, object.numCalls());

    auto concept0 = object.getConcept(0);
    ASSERT_TRUE (concept0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), concept0.getSymbolPath());
    ASSERT_EQ (1, concept0.numImpls());

    auto BoolType = lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Bool")).orElseThrow();

    auto FooUrl = lyric_common::SymbolUrl::fromString("#Foo");
    auto FooType = lyric_common::TypeDef::forConcrete(FooUrl).orElseThrow();

    auto impl0 = concept0.getImpl(0);
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
