#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_compiler_fixture.h"

class CompileTypename : public BaseCompilerFixture {};

TEST_F(CompileTypename, CompileTypenameResolvedToStruct)
{
    auto compileModuleResult = m_tester->compileModule(R"(
        typename Foo
        var x: Foo | Nil = nil
        defstruct Foo{}
    )");
    ASSERT_THAT (compileModuleResult,
        tempo_test::ContainsResult(CompileModule(lyric_build::TaskState::Status::COMPLETED)));

    auto compileModule = compileModuleResult.getResult();
    auto object = compileModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (4, root.numSymbols());

    auto symbol1 = root.findSymbol(lyric_common::SymbolPath::fromString("Foo"));
    ASSERT_TRUE (symbol1.isValid());
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Struct);
    ASSERT_EQ (symbol1.getLinkageIndex(), 0);
}
