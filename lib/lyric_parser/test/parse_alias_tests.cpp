#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_test/result_matchers.h>

#include "base_parser_fixture.h"
#include "lyric_parser/ast_attrs.h"

class ParseAlias : public BaseParserFixture {};

TEST_F(ParseAlias, ParseBindingAlias)
{
    auto parseResult = parseModule(R"(
        alias Alias = Int
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto aliasNode = root.getChild(0);
    ASSERT_TRUE (aliasNode.isClass(lyric_schema::kLyricAstAliasClass));
    ASSERT_LE (0, aliasNode.numChildren());

    std::string identifier;
    ASSERT_THAT (aliasNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("Alias", identifier);

    bool isHidden;
    ASSERT_THAT (aliasNode.parseAttr(lyric_parser::kLyricAstIsHidden, isHidden), tempo_test::IsOk());
    ASSERT_FALSE (isHidden);

    lyric_parser::NodeWalker typeNode;
    aliasNode.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode);
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstSTypeClass));

    lyric_common::SymbolPath symbolPath;
    typeNode.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Int"), symbolPath);
}

TEST_F(ParseAlias, ParseParametricBindingAlias)
{
    auto parseResult = parseModule(R"(
        alias Alias[T] = Foo[T]
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto aliasNode = root.getChild(0);
    ASSERT_TRUE (aliasNode.isClass(lyric_schema::kLyricAstAliasClass));
    ASSERT_LE (0, aliasNode.numChildren());

    std::string identifier;
    ASSERT_THAT (aliasNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("Alias", identifier);

    bool isHidden;
    ASSERT_THAT (aliasNode.parseAttr(lyric_parser::kLyricAstIsHidden, isHidden), tempo_test::IsOk());
    ASSERT_FALSE (isHidden);

    lyric_parser::NodeWalker typeNode;
    aliasNode.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode);
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstPTypeClass));

    lyric_common::SymbolPath symbolPath;
    typeNode.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Foo"), symbolPath);
}

TEST_F(ParseAlias, ParseIndexAlias)
{
    auto parseResult = parseModule(R"(
        alias Alias using Foo[0] = Int
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto aliasNode = root.getChild(0);
    ASSERT_TRUE (aliasNode.isClass(lyric_schema::kLyricAstAliasClass));
    ASSERT_LE (0, aliasNode.numChildren());

    std::string identifier;
    ASSERT_THAT (aliasNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("Alias", identifier);

    bool isHidden;
    ASSERT_THAT (aliasNode.parseAttr(lyric_parser::kLyricAstIsHidden, isHidden), tempo_test::IsOk());
    ASSERT_FALSE (isHidden);

    lyric_common::SymbolPath indexSymbolPath;
    aliasNode.parseAttr(lyric_parser::kLyricAstSymbolPath, indexSymbolPath);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Foo"), indexSymbolPath);

    std::string indexLiteral;
    aliasNode.parseAttr(lyric_parser::kLyricAstLiteralValue, indexLiteral);
    ASSERT_EQ ("0", indexLiteral);

    lyric_parser::NodeWalker typeNode;
    aliasNode.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode);
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstSTypeClass));

    lyric_common::SymbolPath symbolPath;
    typeNode.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Int"), symbolPath);
}

TEST_F(ParseAlias, ParseKeyAlias)
{
    auto parseResult = parseModule(R"(
        alias Alias using T in Foo = Int
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto aliasNode = root.getChild(0);
    ASSERT_TRUE (aliasNode.isClass(lyric_schema::kLyricAstAliasClass));
    ASSERT_LE (0, aliasNode.numChildren());

    std::string identifier;
    ASSERT_THAT (aliasNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("Alias", identifier);

    bool isHidden;
    ASSERT_THAT (aliasNode.parseAttr(lyric_parser::kLyricAstIsHidden, isHidden), tempo_test::IsOk());
    ASSERT_FALSE (isHidden);

    lyric_common::SymbolPath indexSymbolPath;
    aliasNode.parseAttr(lyric_parser::kLyricAstSymbolPath, indexSymbolPath);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Foo"), indexSymbolPath);

    std::string indexLiteral;
    aliasNode.parseAttr(lyric_parser::kLyricAstLiteralValue, indexLiteral);
    ASSERT_EQ ("T", indexLiteral);

    lyric_parser::NodeWalker typeNode;
    aliasNode.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode);
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstSTypeClass));

    lyric_common::SymbolPath symbolPath;
    typeNode.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Int"), symbolPath);
}
