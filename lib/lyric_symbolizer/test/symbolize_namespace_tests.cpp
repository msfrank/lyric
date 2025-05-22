#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_symbolizer_fixture.h"

class SymbolizeNamespace : public BaseSymbolizerFixture {};

TEST_F(SymbolizeNamespace, DeclareNamespace)
{
    auto symbolizeModuleResult = m_tester->symbolizeModule(R"(
        namespace Namespace {
            global val FortyTwo: Int = 42
        }
    )");
    ASSERT_THAT (symbolizeModuleResult,
        tempo_test::ContainsResult(SymbolizeModule(lyric_build::TaskState::Status::COMPLETED)));

    auto symbolizeModule = symbolizeModuleResult.getResult();
    auto object = symbolizeModule.getModule();
    auto root = object.getObject();
    ASSERT_EQ (3, root.numSymbols());
    ASSERT_EQ (1, root.numImports());

    TU_LOG_INFO << object.dumpJson();

    auto symbol1 = root.findSymbol(lyric_common::SymbolPath::fromString("Namespace"));
    ASSERT_EQ (symbol1.getLinkageSection(), lyric_object::LinkageSection::Namespace);
    ASSERT_NE (symbol1.getLinkageIndex(), lyric_object::INVALID_ADDRESS_U32);
}
