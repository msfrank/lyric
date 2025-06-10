#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
#include <tempo_utils/logging.h>

#include "base_parser_fixture.h"

class ParseMacro : public BaseParserFixture {};

TEST_F(ParseMacro, ParseBlockMacro)
{
    auto parseResult = parseModule(R"(
        @{
            Trap("TrapName")
        }
    )");

    ASSERT_TRUE(parseResult.isResult());
}

TEST_F(ParseMacro, ParsePragmaMacro)
{
    auto parseResult = parseModule(R"(
        @@Plugin("/plugin")
        nil
    )");

    ASSERT_TRUE(parseResult.isResult());
}
