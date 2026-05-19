#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_test/result_matchers.h>

#include "base_parser_fixture.h"

class ParseDefClass : public BaseParserFixture {};

TEST_F(ParseDefClass, EmptyClass) {

    auto parseResult = parseModule(R"(
        defclass Foo {
        }
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defclassNode = root.getChild(0);
    ASSERT_TRUE (defclassNode.isClass(lyric_schema::kLyricAstDefClassClass));
    ASSERT_EQ (0, defclassNode.numChildren());

    std::string identifier;
    ASSERT_THAT (defclassNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("Foo", identifier);
}

TEST_F(ParseDefClass, ClassWithEmptyGlobalBlock) {

    auto parseResult = parseModule(R"(
        defclass Foo {
            global {
            }
        }
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defclassNode = root.getChild(0);
    ASSERT_TRUE (defclassNode.isClass(lyric_schema::kLyricAstDefClassClass));
    ASSERT_EQ (1, defclassNode.numChildren());

    auto globalNode = defclassNode.getChild(0);
    ASSERT_TRUE (globalNode.isClass(lyric_schema::kLyricAstGlobalClass));
    ASSERT_EQ (0, globalNode.numChildren());
}

TEST_F(ParseDefClass, ClassWithGlobalVal) {

    auto parseResult = parseModule(R"(
        defclass Foo {
            global {
                val Num: Int = 0
            }
        }
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defclassNode = root.getChild(0);
    ASSERT_TRUE (defclassNode.isClass(lyric_schema::kLyricAstDefClassClass));
    ASSERT_EQ (1, defclassNode.numChildren());

    auto globalNode = defclassNode.getChild(0);
    ASSERT_TRUE (globalNode.isClass(lyric_schema::kLyricAstGlobalClass));
    ASSERT_EQ (1, globalNode.numChildren());

    auto valNode = globalNode.getChild(0);
    ASSERT_TRUE (valNode.isClass(lyric_schema::kLyricAstDefStaticClass));

    std::string identifier;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("Num", identifier);

    bool isVariable;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstIsVariable, isVariable), tempo_test::IsOk());
    ASSERT_FALSE (isVariable);
}

TEST_F(ParseDefClass, ClassWithDecl) {

    auto parseResult = parseModule(R"(
        defclass Foo {
            decl Bar(i: Int): Int
        }
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defclassNode = root.getChild(0);
    ASSERT_TRUE (defclassNode.isClass(lyric_schema::kLyricAstDefClassClass));
    ASSERT_EQ (1, defclassNode.numChildren());

    auto declNode = defclassNode.getChild(0);
    ASSERT_TRUE (declNode.isClass(lyric_schema::kLyricAstDeclClass));
    ASSERT_EQ (1, declNode.numChildren());

    std::string identifier;
    ASSERT_THAT (declNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("Bar", identifier);

    auto packNode = declNode.getChild(0);
    ASSERT_TRUE (packNode.isClass(lyric_schema::kLyricAstPackClass));
    ASSERT_EQ (1, packNode.numChildren());
}
