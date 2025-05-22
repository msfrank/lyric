#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_symbolizer_fixture.h"

class SymbolizeStatic : public BaseSymbolizerFixture {};

TEST_F(SymbolizeStatic, DeclareStaticVal)
{
    auto symbolizeModuleResult = m_tester->symbolizeModule(R"(
        global val Static: Int = 0
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (1, root.numImports());

    auto symbol1 = root.findSymbol(lyric_common::SymbolPath::fromString("Static"));
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Static);
    ASSERT_EQ (symbol1.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);
}

TEST_F(SymbolizeStatic, DeclareStaticVar)
{
    auto symbolizeModuleResult = m_tester->symbolizeModule(R"(
        global var Static: Int = 0
    )");
    ASSERT_THAT (symbolizeModuleResult,
                 tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (1, root.numImports());

    auto symbol1 = root.findSymbol(lyric_common::SymbolPath::fromString("Static"));
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Static);
    ASSERT_EQ (symbol1.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);
}
