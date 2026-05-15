#include <absl/strings/substitute.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_test/result_matchers.h>

#include "base_parser_fixture.h"

struct ImplParameter {
    std::string definitionKeyword;
    tempo_schema::SchemaClass<lyric_schema::LyricAstNs,lyric_schema::LyricAstId> astClass;
};

class ParseImpls : public BaseParserFixture, public ::testing::WithParamInterface<ImplParameter> {};

INSTANTIATE_TEST_SUITE_P(
    ParseImpl,
    ParseImpls,
    ::testing::Values(
        ImplParameter{"defclass", lyric_schema::kLyricAstDefClassClass},
        ImplParameter{"defconcept", lyric_schema::kLyricAstDefConceptClass},
        ImplParameter{"defenum", lyric_schema::kLyricAstDefEnumClass}
        ),
    [](const testing::TestParamInfo<ParseImpls::ParamType>& p) {
      return p.param.definitionKeyword.empty()? absl::StrCat("param#", p.index) : p.param.definitionKeyword;
    }
);

TEST_P(ParseImpls, EmptyImpl)
{
    auto &param = GetParam();

    auto parseResult = parseModule(absl::Substitute(R"(
        $0 Foo {
            impl Identity {
            }
        }
    )", param.definitionKeyword));

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defclassNode = root.getChild(0);
    ASSERT_TRUE (defclassNode.isClass(param.astClass));
    ASSERT_EQ (1, defclassNode.numChildren());

    auto implNode = defclassNode.getChild(0);
    ASSERT_TRUE (implNode.isClass(lyric_schema::kLyricAstImplClass));
    ASSERT_EQ (0, implNode.numChildren());
}

TEST_P(ParseImpls, ImplWithAlias)
{
    auto &param = GetParam();

    auto parseResult = parseModule(absl::Substitute(R"(
        $0 Foo {
            impl Identity {
                alias Type = Int
            }
        }
    )", param.definitionKeyword));

    ASSERT_THAT (parseResult, tempo_test::IsResult());
    auto archetype = parseResult.getResult();
    auto root = archetype.getRoot();
    ASSERT_TRUE (root.isClass(lyric_schema::kLyricAstBlockClass));
    auto defclassNode = root.getChild(0);
    ASSERT_TRUE (defclassNode.isClass(param.astClass));
    ASSERT_EQ (1, defclassNode.numChildren());

    auto implNode = defclassNode.getChild(0);
    ASSERT_TRUE (implNode.isClass(lyric_schema::kLyricAstImplClass));
    ASSERT_EQ (1, implNode.numChildren());

    auto aliasNode = implNode.getChild(0);
    ASSERT_TRUE (aliasNode.isClass(lyric_schema::kLyricAstAliasClass));
}
