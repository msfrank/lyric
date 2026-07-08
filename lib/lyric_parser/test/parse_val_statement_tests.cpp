#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/logging.h>

#include "base_parser_fixture.h"

class ParseValStatement : public BaseParserFixture {};

TEST_F(ParseValStatement, TypedValWithLiteralInitializer)
{
    parserOptions.enableExtraDiagnostics =  true;
    parserOptions.reportAllAmbiguities =  true;

    auto parseResult = parseModule(R"(
        val x: Int = 1
    )");

    ASSERT_THAT(parseResult, tempo_test::IsResult());
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

    auto valNode = block.getChild(0);
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

    ASSERT_THAT(parseResult, tempo_test::IsResult());
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

    auto valNode = block.getChild(0);
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

    ASSERT_THAT(parseResult, tempo_test::IsResult());
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

    auto valNode = block.getChild(0);
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
