#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
#include <tempo_utils/logging.h>


TEST(ParseMacro, ParseBlockMacro)
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

TEST(ParseMacro, ParsePragmaMacro)
{
    lyric_parser::LyricParser parser({});

    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        @@Plugin("/plugin")
        nil
    )", {}, recorder);

    ASSERT_TRUE(parseResult.isResult());
}
