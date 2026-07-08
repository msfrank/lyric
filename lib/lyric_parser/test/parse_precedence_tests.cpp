#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_test/result_matchers.h>

#include "base_parser_fixture.h"

class ParsePrecedence : public BaseParserFixture {};

TEST_F(ParsePrecedence, MultiplicationPrecedesAddition)
{
    auto parseResult = parseModule(R"(
        1 + 2 * 3
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
    ASSERT_EQ (1, block.numChildren());

    auto add = block.getChild(0);
    ASSERT_TRUE (add.isClass(lyric_schema::kLyricAstAddClass));
    ASSERT_EQ (2, add.numChildren());

    auto literal1 = add.getChild(0);
    ASSERT_TRUE (literal1.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal1.numChildren());

    auto mul = add.getChild(1);
    ASSERT_TRUE (mul.isClass(lyric_schema::kLyricAstMulClass));
    ASSERT_EQ (2, mul.numChildren());

    auto literal2 = mul.getChild(0);
    ASSERT_TRUE (literal2.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal2.numChildren());

    auto literal3 = mul.getChild(1);
    ASSERT_TRUE (literal3.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal3.numChildren());

    std::string literalValue;

    ASSERT_THAT (literal1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("1", literalValue);
    ASSERT_THAT (literal2.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("2", literalValue);
    ASSERT_THAT (literal3.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("3", literalValue);
}

TEST_F(ParsePrecedence, DivisionPrecedesAddition)
{
    auto parseResult = parseModule(R"(
        1 + 2 / 3
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
    ASSERT_EQ (1, block.numChildren());

    auto add = block.getChild(0);
    ASSERT_TRUE (add.isClass(lyric_schema::kLyricAstAddClass));
    ASSERT_EQ (2, add.numChildren());

    auto literal1 = add.getChild(0);
    ASSERT_TRUE (literal1.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal1.numChildren());

    auto div = add.getChild(1);
    ASSERT_TRUE (div.isClass(lyric_schema::kLyricAstDivClass));
    ASSERT_EQ (2, div.numChildren());

    auto literal2 = div.getChild(0);
    ASSERT_TRUE (literal2.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal2.numChildren());

    auto literal3 = div.getChild(1);
    ASSERT_TRUE (literal3.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal3.numChildren());

    std::string literalValue;

    ASSERT_THAT (literal1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("1", literalValue);
    ASSERT_THAT (literal2.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("2", literalValue);
    ASSERT_THAT (literal3.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("3", literalValue);
}

TEST_F(ParsePrecedence, MultiplicationPrecedesSubtraction)
{
    auto parseResult = parseModule(R"(
        1 - 2 * 3
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
    ASSERT_EQ (1, block.numChildren());

    auto sub = block.getChild(0);
    ASSERT_TRUE (sub.isClass(lyric_schema::kLyricAstSubClass));
    ASSERT_EQ (2, sub.numChildren());

    auto literal1 = sub.getChild(0);
    ASSERT_TRUE (literal1.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal1.numChildren());

    auto mul = sub.getChild(1);
    ASSERT_TRUE (mul.isClass(lyric_schema::kLyricAstMulClass));
    ASSERT_EQ (2, mul.numChildren());

    auto literal2 = mul.getChild(0);
    ASSERT_TRUE (literal2.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal2.numChildren());

    auto literal3 = mul.getChild(1);
    ASSERT_TRUE (literal3.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal3.numChildren());

    std::string literalValue;

    ASSERT_THAT (literal1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("1", literalValue);
    ASSERT_THAT (literal2.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("2", literalValue);
    ASSERT_THAT (literal3.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("3", literalValue);
}

TEST_F(ParsePrecedence, DivisionPrecedesSubtraction)
{
    auto parseResult = parseModule(R"(
        1 - 2 / 3
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
    ASSERT_EQ (1, block.numChildren());

    auto sub = block.getChild(0);
    ASSERT_TRUE (sub.isClass(lyric_schema::kLyricAstSubClass));
    ASSERT_EQ (2, sub.numChildren());

    auto literal1 = sub.getChild(0);
    ASSERT_TRUE (literal1.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal1.numChildren());

    auto div = sub.getChild(1);
    ASSERT_TRUE (div.isClass(lyric_schema::kLyricAstDivClass));
    ASSERT_EQ (2, div.numChildren());

    auto literal2 = div.getChild(0);
    ASSERT_TRUE (literal2.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal2.numChildren());

    auto literal3 = div.getChild(1);
    ASSERT_TRUE (literal3.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, literal3.numChildren());

    std::string literalValue;

    ASSERT_THAT (literal1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("1", literalValue);
    ASSERT_THAT (literal2.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("2", literalValue);
    ASSERT_THAT (literal3.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("3", literalValue);
}
