#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_symbolizer_fixture.h"

class SymbolizeTypename : public BaseSymbolizerFixture {};

TEST_F(SymbolizeTypename, DeclareTypename)
{
    auto symbolizeModuleResult = m_tester->symbolizeModule(R"(
        typename Foo
        defstruct Foo{}
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (1, root.numImports());

    auto symbol1 = root.findSymbol(lyric_common::SymbolPath::fromString("Foo"));
    ASSERT_TRUE (symbol1.isValid());
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Struct);
    ASSERT_EQ (symbol1.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);
}

TEST_F(SymbolizeTypename, DeclareAliasWithTypenames)
{
    auto symbolizeModuleResult = m_tester->symbolizeModule(R"(
        typename Foo
        typename Bar
        defalias FooOrBar from Foo | Bar
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (5, root.numSymbols());
    ASSERT_EQ (1, root.numImports());

    auto symbol1 = root.findSymbol(lyric_common::SymbolPath::fromString("Foo"));
    ASSERT_TRUE (symbol1.isValid());
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Type);
    ASSERT_EQ (symbol1.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);

    auto symbol2 = root.findSymbol(lyric_common::SymbolPath::fromString("Bar"));
    ASSERT_TRUE (symbol2.isValid());
    ASSERT_EQ (symbol2.getLinkageSection(), lyric_object::LinkageSection::Type);
    ASSERT_EQ (symbol2.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);

    auto symbol3 = root.findSymbol(lyric_common::SymbolPath::fromString("FooOrBar"));
    ASSERT_TRUE (symbol3.isValid());
    ASSERT_EQ (symbol3.getLinkageSection(), lyric_object::LinkageSection::Binding);
    ASSERT_EQ (symbol3.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);
}
