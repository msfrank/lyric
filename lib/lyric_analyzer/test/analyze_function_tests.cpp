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

#include "base_analyzer_fixture.h"

class AnalyzeFunction : public BaseAnalyzerFixture {};

TEST_F(AnalyzeFunction, DeclareFunction)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        def Identity(x: Int): Int { return x }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (2, root.numCalls());

    auto call1 = root.getCall(1);
    ASSERT_TRUE (call1.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Identity"}), call1.getSymbolPath());
    ASSERT_EQ (lyric_object::AccessType::Public, call1.getAccess());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), call1.getResultType().getTypeDef());

    ASSERT_EQ (1, call1.numListParameters());
    ASSERT_EQ (0, call1.numNamedParameters());
    auto param0 = call1.getListParameter(0);
    ASSERT_EQ ("x", param0.getParameterName());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), param0.getParameterType().getTypeDef());
}

TEST_F(AnalyzeFunction, DeclareGenericFunction)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        def Identity[T](x: T): T { return x }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (2, root.numCalls());

    auto call1 = root.getCall(1);
    ASSERT_TRUE (call1.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Identity"}), call1.getSymbolPath());
    ASSERT_EQ (lyric_object::AccessType::Public, call1.getAccess());
    ASSERT_EQ (lyric_common::TypeDef::forPlaceholder(0, lyric_common::SymbolUrl::fromString("#Identity")), call1.getResultType().getTypeDef());

    ASSERT_EQ (1, call1.numListParameters());
    ASSERT_EQ (0, call1.numNamedParameters());
    auto param0 = call1.getListParameter(0);
    ASSERT_EQ ("x", param0.getParameterName());
    ASSERT_EQ (lyric_common::TypeDef::forPlaceholder(0, lyric_common::SymbolUrl::fromString("#Identity")), param0.getParameterType().getTypeDef());
}
