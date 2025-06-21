#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <tempo_test/result_matchers.h>

#include "base_parser_fixture.h"
#include "lyric_parser/ast_attrs.h"
#include "lyric_schema/ast_schema.h"

class ParseType : public BaseParserFixture {};

TEST_F(ParseType, ParseSimpleTypeSingleIdentifier) {

    auto parseResult = parseModule(R"(
        val foo: Foo = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto child1 = root.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstValClass));

    lyric_parser::NodeWalker typeNode;
    child1.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode);
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstSTypeClass));

    lyric_common::SymbolPath symbolPath;
    typeNode.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Foo"), symbolPath);
}

TEST_F(ParseType, ParseSimpleTypeMultipleIdentifiers) {

    auto parseResult = parseModule(R"(
        val foo: Foo.Bar.Baz = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto child1 = root.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstValClass));

    lyric_parser::NodeWalker typeNode;
    child1.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode);
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstSTypeClass));

    lyric_common::SymbolPath symbolPath;
    typeNode.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Foo.Bar.Baz"), symbolPath);
}

TEST_F(ParseType, ParseParametricTypeSingleParameter) {

    auto parseResult = parseModule(R"(
        val foo: Foo[T] = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto child1 = root.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstValClass));

    lyric_parser::NodeWalker typeNode;
    child1.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode);
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstPTypeClass));
    ASSERT_EQ (1, typeNode.numChildren());

    auto typeParamNode = typeNode.getChild(0);
    lyric_common::SymbolPath symbolPath;
    typeParamNode.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("T"), symbolPath);
}

TEST_F(ParseType, ParseParametricTypeMultipleParameters) {

    auto parseResult = parseModule(R"(
        val foo: Foo[T,U,V] = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto child1 = root.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstValClass));

    lyric_parser::NodeWalker typeNode;
    child1.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode);
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstPTypeClass));
    ASSERT_EQ (3, typeNode.numChildren());

    auto typeParamNode1 = typeNode.getChild(0);
    lyric_common::SymbolPath symbolPath1;
    typeParamNode1.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath1);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("T"), symbolPath1);

    auto typeParamNode2 = typeNode.getChild(1);
    lyric_common::SymbolPath symbolPath2;
    typeParamNode2.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath2);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("U"), symbolPath2);

    auto typeParamNode3 = typeNode.getChild(2);
    lyric_common::SymbolPath symbolPath3;
    typeParamNode3.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath3);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("V"), symbolPath3);
}

TEST_F(ParseType, ParseUnionType) {

    auto parseResult = parseModule(R"(
        val foo: Foo | Bar | Baz = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto child1 = root.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstValClass));

    lyric_parser::NodeWalker typeNode;
    child1.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode);
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstUTypeClass));
    ASSERT_EQ (3, typeNode.numChildren());

    auto typeMemberNode1 = typeNode.getChild(0);
    lyric_common::SymbolPath symbolPath1;
    typeMemberNode1.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath1);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Foo"), symbolPath1);

    auto typeMemberNode2 = typeNode.getChild(1);
    lyric_common::SymbolPath symbolPath2;
    typeMemberNode2.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath2);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Bar"), symbolPath2);

    auto typeMemberNode3 = typeNode.getChild(2);
    lyric_common::SymbolPath symbolPath3;
    typeMemberNode3.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath3);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Baz"), symbolPath3);
}

TEST_F(ParseType, ParseIntersectionType) {

    auto parseResult = parseModule(R"(
        val foo: Foo & Bar & Baz = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto child1 = root.getChild(0);
    ASSERT_TRUE (child1.isClass(lyric_schema::kLyricAstValClass));

    lyric_parser::NodeWalker typeNode;
    child1.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode);
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstITypeClass));
    ASSERT_EQ (3, typeNode.numChildren());

    auto typeMemberNode1 = typeNode.getChild(0);
    lyric_common::SymbolPath symbolPath1;
    typeMemberNode1.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath1);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Foo"), symbolPath1);

    auto typeMemberNode2 = typeNode.getChild(1);
    lyric_common::SymbolPath symbolPath2;
    typeMemberNode2.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath2);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Bar"), symbolPath2);

    auto typeMemberNode3 = typeNode.getChild(2);
    lyric_common::SymbolPath symbolPath3;
    typeMemberNode3.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath3);
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Baz"), symbolPath3);
}

TEST_F(ParseType, ParseFailsSimpleTypeExtraDotAfterIdentifier) {

    auto parseResult = parseModule(R"(
        val foo: Foo.Bar. = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Extra '.' after identifier in the type path"));
}

TEST_F(ParseType, ParseFailsParametricTypeExtraBracketClosingType) {

    auto parseResult = parseModule(R"(
        val foo: Foo[Int]] = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Extra ']' closing parametric type"));
}

TEST_F(ParseType, ParseFailsParametricTypeExtraCommaBetweenTypeParameters) {

    auto parseResult = parseModule(R"(
        val foo: Foo[Int,,Int] = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Extra ',' after type parameter in the parametric type"));
}

TEST_F(ParseType, ParseFailsParametricTypeExtraCommaAfterLastTypeParameter) {

    auto parseResult = parseModule(R"(
        val foo: Foo[Int,Int,] = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Extra ',' after type parameter in the parametric type"));
}

TEST_F(ParseType, ParseFailsParametricTypeExtraCommaBeforeFirstTypeParameter) {

    auto parseResult = parseModule(R"(
        val foo: Foo[,Int,Int] = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Extra ',' before type parameter in the parametric type"));
}

TEST_F(ParseType, ParseFailsParametricTypeMissingCommaBetweenTypeParameters) {

    auto parseResult = parseModule(R"(
        val foo: Foo[Int Int] = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Missing ',' between type parameters in the parametric type"));
}

TEST_F(ParseType, ParseFailsParametricTypeHasNoTypeParameters) {

    auto parseResult = parseModule(R"(
        val foo: Foo[] = 42
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Parametric type must contain at least one type parameter"));
}
