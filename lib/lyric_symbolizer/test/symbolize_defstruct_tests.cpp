#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_symbolizer_fixture.h"

class SymbolizeDefstruct : public BaseSymbolizerFixture {};

TEST_F(SymbolizeDefstruct, DeclareDefstruct)
{
    auto symbolizeModuleResult = m_tester->symbolizeModule(R"(
        defstruct Struct {
        }
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getModule();
    ASSERT_EQ (3, object.numSymbols());
    ASSERT_EQ (1, object.numImports());

    auto symbol1 = object.findSymbol(lyric_common::SymbolPath::fromString("Struct"));
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Struct);
    ASSERT_EQ (symbol1.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);
}

TEST_F(SymbolizeDefstruct, DeclareStructDefaultInit)
{
    auto symbolizeModuleResult = m_tester->symbolizeModule(R"(
        defstruct Struct {
            init() {
            }
        }
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getModule();
    ASSERT_EQ (4, object.numSymbols());
    ASSERT_EQ (1, object.numImports());

    auto symbol1 = object.findSymbol(lyric_common::SymbolPath::fromString("Struct.$ctor"));
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Call);
    ASSERT_EQ (symbol1.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);
}

TEST_F(SymbolizeDefstruct, DeclareStructNamedInit)
{
    auto symbolizeModuleResult = m_tester->symbolizeModule(R"(
        defstruct Struct {
            init Named() {
            }
        }
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getModule();
    ASSERT_EQ (4, object.numSymbols());
    ASSERT_EQ (1, object.numImports());

    auto symbol1 = object.findSymbol(lyric_common::SymbolPath::fromString("Struct.Named"));
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Call);
    ASSERT_EQ (symbol1.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);
}
