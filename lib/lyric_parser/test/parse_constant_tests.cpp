#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_test/result_matchers.h>

#include "base_parser_fixture.h"

class ParseConstant : public BaseParserFixture {};

TEST_F(ParseConstant, ParseDecimalInteger) {

    auto parseResult = parseModule(R"(
        123
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

    auto child1 = block.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstIntegerClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("123", literalValue);
    lyric_common::NumericBase base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::NumericBase::Decimal, base);
}

TEST_F(ParseConstant, ParseNegativeDecimalInteger) {

    auto parseResult = parseModule(R"(
        -123
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

    auto child1 = block.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstIntegerClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("-123", literalValue);
    lyric_common::NumericBase base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::NumericBase::Decimal, base);
}

TEST_F(ParseConstant, ParseDecimalFixedFloat) {

    auto parseResult = parseModule(R"(
        123.456
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

    auto child1 = block.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstFloatClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("123.456", literalValue);
    lyric_common::NumericBase base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::NumericBase::Decimal, base);
    bool scientific;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstIsScientific, scientific), tempo_test::IsOk());
    ASSERT_FALSE (scientific);
}

TEST_F(ParseConstant, ParseNegativeDecimalFixedFloat) {

    auto parseResult = parseModule(R"(
        -123.456
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

    auto child1 = block.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstFloatClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("-123.456", literalValue);
    lyric_common::NumericBase base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::NumericBase::Decimal, base);
    bool scientific;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstIsScientific, scientific), tempo_test::IsOk());
    ASSERT_FALSE (scientific);
}

TEST_F(ParseConstant, ParseDecimalScientificFloat) {

    auto parseResult = parseModule(R"(
        123.456e7
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

    auto child1 = block.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstFloatClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("123.456e7", literalValue);
    lyric_common::NumericBase base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::NumericBase::Decimal, base);
    bool scientific;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstIsScientific, scientific), tempo_test::IsOk());
    ASSERT_TRUE (scientific);
}

TEST_F(ParseConstant, ParseNegativeDecimalScientificFloat) {

    auto parseResult = parseModule(R"(
        -123.456e7
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

    auto child1 = block.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstFloatClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("-123.456e7", literalValue);
    lyric_common::NumericBase base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::NumericBase::Decimal, base);
    bool scientific;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstIsScientific, scientific), tempo_test::IsOk());
    ASSERT_TRUE (scientific);
}

TEST_F(ParseConstant, ParseHexInteger) {

    auto parseResult = parseModule(R"(
        0x123
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

    auto child1 = block.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstIntegerClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("123", literalValue);
    lyric_common::NumericBase base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::NumericBase::Hex, base);
}

TEST_F(ParseConstant, ParseNegativeHexInteger) {

    auto parseResult = parseModule(R"(
        -0x123
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

    auto child1 = block.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstIntegerClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("-123", literalValue);
    lyric_common::NumericBase base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::NumericBase::Hex, base);
}

TEST_F(ParseConstant, ParseOctalInteger) {

    auto parseResult = parseModule(R"(
        0o123
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

    auto child1 = block.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstIntegerClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("123", literalValue);
    lyric_common::NumericBase base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::NumericBase::Octal, base);
}

TEST_F(ParseConstant, ParseNegativeOctalInteger) {

    auto parseResult = parseModule(R"(
        -0o123
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

    auto child1 = block.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstIntegerClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("-123", literalValue);
    lyric_common::NumericBase base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::NumericBase::Octal, base);
}

TEST_F(ParseConstant, InvalidHexIntegerIsSyntaxError) {

    auto parseResult = parseModule(R"(
        0xq123
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
}

TEST_F(ParseConstant, InvalidOctalIntegerIsSyntaxError) {

    auto parseResult = parseModule(R"(
        0oq123
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
}
