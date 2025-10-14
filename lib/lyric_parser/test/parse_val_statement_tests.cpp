#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/logging.h>

#include "base_parser_fixture.h"

class ParseValStatement : public BaseParserFixture {};

TEST_F(ParseValStatement, TypedValWithLiteralInitializer)
{
    auto parseResult = parseModule(R"(
        val x: Int = 1
    )");

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    auto blockNode = archetype.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (1, blockNode.numChildren());

    auto valNode = blockNode.getChild(0);
    ASSERT_TRUE (valNode.isClass(lyric_schema::kLyricAstValClass));
    ASSERT_EQ (3, valNode.numAttrs());
    ASSERT_EQ (1, valNode.numChildren());

    std::string identifier;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("x", identifier);

    bool isHidden;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstIsHidden, isHidden), tempo_test::IsOk());
    ASSERT_FALSE (isHidden);

    lyric_parser::NodeWalker typeNode;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode), tempo_test::IsOk());
    ASSERT_TRUE (typeNode.isValid());

    lyric_common::SymbolPath typePath;
    ASSERT_THAT (typeNode.parseAttr(lyric_parser::kLyricAstSymbolPath, typePath), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Int"), typePath);

    auto intNode = valNode.getChild(0);
    ASSERT_TRUE (intNode.isClass(lyric_schema::kLyricAstIntegerClass));

    std::string literalValue;
    ASSERT_THAT (intNode.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("1", literalValue);
}

TEST_F(ParseValStatement, TypedValWithDefaultNewInitializer)
{
    auto parseResult = parseModule(R"(
        val x: Foo = {}
    )");

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    auto blockNode = archetype.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (1, blockNode.numChildren());

    auto valNode = blockNode.getChild(0);
    ASSERT_TRUE (valNode.isClass(lyric_schema::kLyricAstValClass));
    ASSERT_EQ (3, valNode.numAttrs());
    ASSERT_EQ (1, valNode.numChildren());

    std::string identifier;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("x", identifier);

    bool isHidden;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstIsHidden, isHidden), tempo_test::IsOk());
    ASSERT_FALSE (isHidden);

    lyric_parser::NodeWalker typeNode;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode), tempo_test::IsOk());
    ASSERT_TRUE (typeNode.isValid());

    lyric_common::SymbolPath typePath;
    ASSERT_THAT (typeNode.parseAttr(lyric_parser::kLyricAstSymbolPath, typePath), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Foo"), typePath);

    auto newNode = valNode.getChild(0);
    ASSERT_TRUE (newNode.isClass(lyric_schema::kLyricAstNewClass));
    ASSERT_EQ (0, newNode.numChildren());
    ASSERT_FALSE (newNode.hasAttr(lyric_parser::kLyricAstSymbolPath));
    ASSERT_FALSE (newNode.hasAttr(lyric_parser::kLyricAstTypeArgumentsOffset));
}

TEST_F(ParseValStatement, UntypedValWithLiteralInitializer)
{
    auto parseResult = parseModule(R"(
        val x = 1
    )");

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    auto blockNode = archetype.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (1, blockNode.numChildren());

    auto valNode = blockNode.getChild(0);
    ASSERT_TRUE (valNode.isClass(lyric_schema::kLyricAstValClass));
    ASSERT_EQ (2, valNode.numAttrs());
    ASSERT_EQ (1, valNode.numChildren());

    std::string identifier;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("x", identifier);

    bool isHidden;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstIsHidden, isHidden), tempo_test::IsOk());
    ASSERT_FALSE (isHidden);

    ASSERT_FALSE (valNode.hasAttr(lyric_parser::kLyricAstTypeOffset));

    auto intNode = valNode.getChild(0);
    ASSERT_TRUE (intNode.isClass(lyric_schema::kLyricAstIntegerClass));

    std::string literalValue;
    ASSERT_THAT (intNode.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("1", literalValue);
}
