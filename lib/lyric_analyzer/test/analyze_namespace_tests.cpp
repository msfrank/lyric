#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_analyzer_fixture.h"

class AnalyzeNamespace : public BaseAnalyzerFixture {};

TEST_F(AnalyzeNamespace, DeclareNamespace)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        namespace Foo {
            global val FortyTwo: Int = 42
        }
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    ASSERT_EQ (3, object.numSymbols());
    ASSERT_EQ (2, object.numNamespaces());

    auto ns1 = object.getNamespace(1);
    ASSERT_FALSE (ns1.isDeclOnly());
    ASSERT_EQ (lyric_common::SymbolPath({"Foo"}), ns1.getSymbolPath());
    ASSERT_EQ (lyric_object::AccessType::Public, ns1.getAccess());
    ASSERT_EQ (0, ns1.numSymbols());
}
