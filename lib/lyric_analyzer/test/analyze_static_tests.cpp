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

class AnalyzeStatic : public BaseAnalyzerFixture {};

TEST_F(AnalyzeStatic, DeclareStaticVal)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        global val Static: Int = 0
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (1, root.numStatics());

    auto static0 = root.getStatic(0);
    ASSERT_EQ (lyric_common::SymbolPath({"Static"}), static0.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), static0.getStaticType().getTypeDef());
    ASSERT_FALSE (static0.isVariable());
}

TEST_F(AnalyzeStatic, DeclareStaticVar)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        global var Static: Int = 0
    )");
    ASSERT_THAT (analyzeModuleResult,
                 tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    for (int i = 0; i < root.numSymbols(); i++) {
        auto symbol = root.getSymbol(i);
        TU_LOG_INFO << "symbol " << symbol.getSymbolPath().toString();
    }
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (1, root.numStatics());

    auto static0 = root.getStatic(0);
    ASSERT_TRUE (static0.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Static"}), static0.getSymbolPath());
    ASSERT_EQ (lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), static0.getStaticType().getTypeDef());
    ASSERT_TRUE (static0.isVariable());
}
