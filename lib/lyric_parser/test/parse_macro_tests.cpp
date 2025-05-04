#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
#include <tempo_utils/logging.h>


TEST(ParseMacro, ParseMacro)
{
    lyric_parser::LyricParser parser({});

    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        @{
            Trap("TrapName")
        }
    )", {}, recorder);

    ASSERT_TRUE(parseResult.isResult());
}

TEST(ParseMacro, ParsePragma)
{
    lyric_parser::LyricParser parser({});

    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        @@Plugin("/plugin")
    )", {}, recorder);

    ASSERT_TRUE(parseResult.isResult());
}
