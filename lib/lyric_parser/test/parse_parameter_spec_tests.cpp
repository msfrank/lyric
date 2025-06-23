#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_test/result_matchers.h>

#include "base_parser_fixture.h"
#include "lyric_parser/ast_attrs.h"

class ParseParameterSpec : public BaseParserFixture {};

TEST_F(ParseParameterSpec, ParseEmptyParameterList) {

    auto parseResult = parseModule(R"(
        def Foo() {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defNode = root.getChild(0);
    ASSERT_TRUE (defNode.isClass(lyric_schema::kLyricAstDefClass));
    ASSERT_LE (1, defNode.numChildren());

    auto packNode = defNode.getChild(0);
    ASSERT_EQ (0, packNode.numChildren());
}

TEST_F(ParseParameterSpec, ParseSinglePositionalParameter) {

    auto parseResult = parseModule(R"(
        def Foo(bar: Int) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defNode = root.getChild(0);
    ASSERT_TRUE (defNode.isClass(lyric_schema::kLyricAstDefClass));
    ASSERT_LE (1, defNode.numChildren());

    auto packNode = defNode.getChild(0);
    ASSERT_EQ (1, packNode.numChildren());

    auto param1 = packNode.getChild(0);
    ASSERT_TRUE (param1.isClass(lyric_schema::kLyricAstParamClass));
    std::string name1;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstIdentifier, name1), tempo_test::IsOk());
    ASSERT_EQ ("bar", name1);
    bool isVariable;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstIsVariable, isVariable), tempo_test::IsOk());
    ASSERT_FALSE (isVariable);
}

TEST_F(ParseParameterSpec, ParseSingleNamedParameter) {

    auto parseResult = parseModule(R"(
        def Foo(named bar: Int) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defNode = root.getChild(0);
    ASSERT_TRUE (defNode.isClass(lyric_schema::kLyricAstDefClass));
    ASSERT_LE (1, defNode.numChildren());

    auto packNode = defNode.getChild(0);
    ASSERT_EQ (1, packNode.numChildren());

    auto param1 = packNode.getChild(0);
    ASSERT_TRUE (param1.isClass(lyric_schema::kLyricAstParamClass));
    std::string name1;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstIdentifier, name1), tempo_test::IsOk());
    ASSERT_EQ ("bar", name1);
    std::string label1;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstLabel, label1), tempo_test::IsOk());
    ASSERT_EQ (name1, label1);
    bool isVariable;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstIsVariable, isVariable), tempo_test::IsOk());
    ASSERT_FALSE (isVariable);
}

TEST_F(ParseParameterSpec, ParseSingleCtxParameter) {

    auto parseResult = parseModule(R"(
        def Foo(using bar: Equality[Int]) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defNode = root.getChild(0);
    ASSERT_TRUE (defNode.isClass(lyric_schema::kLyricAstDefClass));
    ASSERT_LE (1, defNode.numChildren());

    auto packNode = defNode.getChild(0);
    ASSERT_EQ (1, packNode.numChildren());

    auto param1 = packNode.getChild(0);
    ASSERT_TRUE (param1.isClass(lyric_schema::kLyricAstCtxClass));
    std::string name1;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstIdentifier, name1), tempo_test::IsOk());
    ASSERT_EQ ("bar", name1);
    std::string label1;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstLabel, label1), tempo_test::IsOk());
    ASSERT_EQ (name1, label1);
}

TEST_F(ParseParameterSpec, ParseRestParameter) {

    auto parseResult = parseModule(R"(
        def Foo(bar: ...Int) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defNode = root.getChild(0);
    ASSERT_TRUE (defNode.isClass(lyric_schema::kLyricAstDefClass));
    ASSERT_LE (1, defNode.numChildren());

    auto packNode = defNode.getChild(0);
    ASSERT_EQ (0, packNode.numChildren());

    lyric_parser::NodeWalker restNode;
    packNode.parseAttr(lyric_parser::kLyricAstRestOffset, restNode);
    ASSERT_TRUE (restNode.isClass(lyric_schema::kLyricAstRestClass));
    std::string name1;
    ASSERT_THAT (restNode.parseAttr(lyric_parser::kLyricAstIdentifier, name1), tempo_test::IsOk());
    ASSERT_EQ ("bar", name1);
}

TEST_F(ParseParameterSpec, ParsePositionalParameterWithLiteralInitializer) {

    auto parseResult = parseModule(R"(
        def Foo(bar: Int = 42) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defNode = root.getChild(0);
    ASSERT_TRUE (defNode.isClass(lyric_schema::kLyricAstDefClass));
    ASSERT_LE (1, defNode.numChildren());

    auto packNode = defNode.getChild(0);
    ASSERT_EQ (1, packNode.numChildren());

    auto param1 = packNode.getChild(0);
    ASSERT_TRUE (param1.isClass(lyric_schema::kLyricAstParamClass));
    std::string name1;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstIdentifier, name1), tempo_test::IsOk());
    ASSERT_EQ ("bar", name1);

    lyric_parser::NodeWalker defaultNode;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstDefaultOffset, defaultNode), tempo_test::IsOk());
    ASSERT_TRUE (defaultNode.isClass(lyric_schema::kLyricAstIntegerClass));
}

TEST_F(ParseParameterSpec, ParsePositionalParameterWithNewInitializer) {

    auto parseResult = parseModule(R"(
        def Foo(bar: Bar = {1, 2, 3}) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defNode = root.getChild(0);
    ASSERT_TRUE (defNode.isClass(lyric_schema::kLyricAstDefClass));
    ASSERT_LE (1, defNode.numChildren());

    auto packNode = defNode.getChild(0);
    ASSERT_EQ (1, packNode.numChildren());

    auto param1 = packNode.getChild(0);
    ASSERT_TRUE (param1.isClass(lyric_schema::kLyricAstParamClass));
    std::string name1;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstIdentifier, name1), tempo_test::IsOk());
    ASSERT_EQ ("bar", name1);

    lyric_parser::NodeWalker defaultNode;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstDefaultOffset, defaultNode), tempo_test::IsOk());
    ASSERT_TRUE (defaultNode.isClass(lyric_schema::kLyricAstNewClass));
    ASSERT_EQ (3, defaultNode.numChildren());
}

TEST_F(ParseParameterSpec, ParsePositionalParameterWithTypedNewInitializer) {

    auto parseResult = parseModule(R"(
        def Foo(bar: Bar = Bar{1, 2, 3}) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defNode = root.getChild(0);
    ASSERT_TRUE (defNode.isClass(lyric_schema::kLyricAstDefClass));
    ASSERT_LE (1, defNode.numChildren());

    auto packNode = defNode.getChild(0);
    ASSERT_EQ (1, packNode.numChildren());

    auto param1 = packNode.getChild(0);
    ASSERT_TRUE (param1.isClass(lyric_schema::kLyricAstParamClass));
    std::string name1;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstIdentifier, name1), tempo_test::IsOk());
    ASSERT_EQ ("bar", name1);

    lyric_parser::NodeWalker defaultNode;
    ASSERT_THAT (param1.parseAttr(lyric_parser::kLyricAstDefaultOffset, defaultNode), tempo_test::IsOk());
    ASSERT_TRUE (defaultNode.isClass(lyric_schema::kLyricAstNewClass));
    ASSERT_EQ (3, defaultNode.numChildren());

    lyric_parser::NodeWalker typeNode;
    ASSERT_THAT (defaultNode.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode), tempo_test::IsOk());
    ASSERT_TRUE (typeNode.isClass(lyric_schema::kLyricAstSTypeClass));
}

TEST_F(ParseParameterSpec, ParseFailsExtraCommaBetweenParameters) {

    auto parseResult = parseModule(R"(
        def Foo(foo: Int, bar: Int,, baz: Int) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Extra ',' after parameter in the parameter list"));
}

TEST_F(ParseParameterSpec, ParseFailsExtraCommaBeforeFirstParameter) {

    auto parseResult = parseModule(R"(
        def Foo(, foo: Int) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Extra ',' before parameter in the parameter list"));
}

TEST_F(ParseParameterSpec, ParseFailsRestParameterIsNotLast) {

    auto parseResult = parseModule(R"(
        def Foo(foo: ...Int, bar: Int) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Rest parameter must be the last parameter in the parameter list"));
}

TEST_F(ParseParameterSpec, ParseFailsExtraParenAfterParameterList) {

    auto parseResult = parseModule(R"(
        def Foo(bar: Int)) {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Extra ')' closing parameter list"));
}

TEST_F(ParseParameterSpec, ParseFailsMissingParenAfterParameterList) {

    auto parseResult = parseModule(R"(
        def Foo(bar: Int {}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Missing ')' closing parameter list"));
}
