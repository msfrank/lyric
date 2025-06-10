#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <tempo_utils/logging.h>

#include "base_parser_fixture.h"

class ParseArchetype : public BaseParserFixture {};

TEST_F(ParseArchetype, ParseTrueConstant)
{
    auto parseResult = parseModule(R"(
        true
    )");

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    ASSERT_EQ (2, archetype.numNodes());
    auto block = archetype.getRoot();
    ASSERT_TRUE (block.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (1, block.numChildren());
    auto child = block.getChild(0);
    ASSERT_TRUE (child.isClass(lyric_schema::kLyricAstTrueClass));
}

TEST_F(ParseArchetype, ParseStringLiteral)
{
    auto parseResult = parseModule(R"(
        "hello world!"
    )");

    ASSERT_TRUE(parseResult.isResult());
}

TEST_F(ParseArchetype, ParseAddIntegers)
{
    auto parseResult = parseModule(R"(
        1 + 0xF2
    )");

    ASSERT_TRUE(parseResult.isResult());
}
