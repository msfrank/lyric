#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <tempo_utils/logging.h>

TEST(ParseArchetype, ParseTrueConstant)
{
    lyric_parser::LyricParser parser({});

    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        true
    )", {}, recorder);

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    ASSERT_EQ (2, archetype.numNodes());
    auto block = archetype.getNode(0);
    ASSERT_TRUE (block.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (1, block.numChildren());
    auto child = block.getChild(0);
    ASSERT_TRUE (child.isClass(lyric_schema::kLyricAstTrueClass));
}

TEST(ParseArchetype, ParseStringLiteral)
{
    lyric_parser::LyricParser parser({});

    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        "hello world!"
    )", {}, recorder);

    ASSERT_TRUE(parseResult.isResult());

    //TU_CONSOLE_ERR << dump_lyric_archetype(parseResult.getResult());
}

TEST(ParseArchetype, ParseAddIntegers)
{
    lyric_parser::LyricParser parser({});

    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        1 + 0xF2
    )", {}, recorder);

    ASSERT_TRUE(parseResult.isResult());

    //TU_CONSOLE_ERR << dump_lyric_archetype(parseResult.getResult());
}
