#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/logging.h>

TEST(ParseValStatement, ParseTypedVal)
{
    lyric_parser::LyricParser parser({});
    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        val x: Int = 1
    )", {}, recorder);

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    auto blockNode = archetype.getNode(0);
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (1, blockNode.numChildren());

    auto valNode = blockNode.getChild(0);
    ASSERT_TRUE (valNode.isClass(lyric_schema::kLyricAstValClass));
    ASSERT_EQ (2, valNode.numAttrs());
    ASSERT_EQ (1, valNode.numChildren());

    std::string identifier;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstIdentifier, identifier), tempo_test::IsOk());
    ASSERT_EQ ("x", identifier);

    lyric_parser::NodeWalker typeNode;
    ASSERT_THAT (valNode.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode), tempo_test::IsOk());
    ASSERT_TRUE (typeNode.isValid());

    lyric_common::SymbolPath typePath;
    ASSERT_THAT (typeNode.parseAttr(lyric_parser::kLyricAstSymbolPath, typePath), tempo_test::IsOk());
    ASSERT_EQ (lyric_common::SymbolPath::fromString("Int"), typePath);
}
