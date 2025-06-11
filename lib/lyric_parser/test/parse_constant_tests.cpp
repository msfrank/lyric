#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <tempo_test/result_matchers.h>
#include <tempo_tracing/error_walker.h>

#include "base_parser_fixture.h"
#include "lyric_parser/ast_attrs.h"
#include "lyric_schema/assembler_schema.h"
#include "lyric_schema/ast_schema.h"

class ParseConstant : public BaseParserFixture {};

TEST_F(ParseConstant, ParseDecimalInteger) {

    auto parseResult = parseModule(R"(
        123
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));

    auto child1 = root.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstIntegerClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("123", literalValue);
    lyric_parser::BaseType base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_parser::BaseType::Decimal, base);
    lyric_parser::NotationType notation;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstNotationType, notation), tempo_test::IsOk());
    ASSERT_EQ (lyric_parser::NotationType::Integral, notation);
}

TEST_F(ParseConstant, ParseHexInteger) {

    auto parseResult = parseModule(R"(
        0x123
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));

    auto child1 = root.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstIntegerClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("123", literalValue);
    lyric_parser::BaseType base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_parser::BaseType::Hex, base);
    lyric_parser::NotationType notation;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstNotationType, notation), tempo_test::IsOk());
    ASSERT_EQ (lyric_parser::NotationType::Integral, notation);
}

TEST_F(ParseConstant, ParseOctalInteger) {

    auto parseResult = parseModule(R"(
        0o123
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));

    auto child1 = root.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstIntegerClass));
    std::string literalValue;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("123", literalValue);
    lyric_parser::BaseType base;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstBaseType, base), tempo_test::IsOk());
    ASSERT_EQ (lyric_parser::BaseType::Octal, base);
    lyric_parser::NotationType notation;
    ASSERT_THAT (child1.parseAttr(lyric_parser::kLyricAstNotationType, notation), tempo_test::IsOk());
    ASSERT_EQ (lyric_parser::NotationType::Integral, notation);
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
