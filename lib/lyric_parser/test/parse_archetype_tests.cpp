#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <tempo_test/tempo_test.h>
#include <tempo_utils/logging.h>

#include "base_parser_fixture.h"

class ParseArchetype : public BaseParserFixture {};

TEST_F(ParseArchetype, ParseImplicitEntry)
{
    auto parseResult = parseModule(R"(
        true
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();

    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstRootClass));
    ASSERT_EQ (0, root.numChildren());

    lyric_parser::NodeWalker entry;
    ASSERT_THAT (root.parseAttr(lyric_parser::kLyricAstEntryOffset, entry), tempo_test::IsOk());
    ASSERT_TRUE (entry.isClass(lyric_schema::kLyricAstEntryClass));
    ASSERT_EQ (1, entry.numChildren());

    auto block = entry.getChild(0);
    ASSERT_TRUE (block.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (1, block.numChildren());

    auto form = block.getChild(0);
    ASSERT_TRUE (form.isClass(lyric_schema::kLyricAstTrueClass));
}

TEST_F(ParseArchetype, ParseExplicitEntry)
{
    auto parseResult = parseModule(R"(
        init {
            true
        }
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();

    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstRootClass));
    ASSERT_EQ (0, root.numChildren());

    lyric_parser::NodeWalker entry;
    ASSERT_THAT (root.parseAttr(lyric_parser::kLyricAstEntryOffset, entry), tempo_test::IsOk());
    ASSERT_TRUE (entry.isClass(lyric_schema::kLyricAstEntryClass));
    ASSERT_EQ (1, entry.numChildren());

    auto block = entry.getChild(0);
    ASSERT_TRUE (block.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (1, block.numChildren());

    auto form = block.getChild(0);
    ASSERT_TRUE (form.isClass(lyric_schema::kLyricAstTrueClass));
}
