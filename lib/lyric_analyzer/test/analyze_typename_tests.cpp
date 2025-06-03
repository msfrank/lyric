#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_analyzer_fixture.h"

class AnalyzeTypename : public BaseAnalyzerFixture {};

TEST_F(AnalyzeTypename, DeclareTypename)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        typename Foo
        var x: Foo | Nil = nil
        defstruct Foo{}
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (4, root.numSymbols());

    auto symbol1 = root.findSymbol(lyric_common::SymbolPath::fromString("Foo"));
    ASSERT_TRUE (symbol1.isValid());
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Struct);
    ASSERT_EQ (symbol1.getLinkageIndex(), 0);
}

TEST_F(AnalyzeTypename, DeclareAliasWithTypenames)
{
    auto analyzeModuleResult = m_tester->analyzeModule(R"(
        typename Foo
        typename Bar
        defalias FooOrBar from Foo | Bar
        var x: FooOrBar | Nil = nil
        defstruct Foo{}
        defstruct Bar{}
    )");
    ASSERT_THAT (analyzeModuleResult,
        tempo_test::ContainsResult(AnalyzeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto analyzeModule = analyzeModuleResult.getResult();
    auto object = analyzeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (7, root.numSymbols());
    ASSERT_EQ (1, root.numImports());

    auto symbol1 = root.findSymbol(lyric_common::SymbolPath::fromString("Foo"));
    ASSERT_TRUE (symbol1.isValid());
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Struct);
    ASSERT_NE (symbol1.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);

    auto symbol2 = root.findSymbol(lyric_common::SymbolPath::fromString("Bar"));
    ASSERT_TRUE (symbol2.isValid());
    ASSERT_EQ (symbol2.getLinkageSection(), lyric_object::LinkageSection::Struct);
    ASSERT_NE (symbol2.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);

    auto symbol3 = root.findSymbol(lyric_common::SymbolPath::fromString("FooOrBar"));
    ASSERT_TRUE (symbol3.isValid());
    ASSERT_EQ (symbol3.getLinkageSection(), lyric_object::LinkageSection::Binding);
    ASSERT_NE (symbol3.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);
}
