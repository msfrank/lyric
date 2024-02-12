#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
//#include <lyric_parser/dump_lyric_archetype.h>
#include <tempo_utils/logging.h>

TEST(ParseArchetype, ParseTrueConstant)
{
    lyric_parser::LyricParser parser({});

    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        true
    )", {}, recorder);

    ASSERT_TRUE(parseResult.isResult());

    //TU_CONSOLE_ERR << dump_lyric_archetype(parseResult.getResult());
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
