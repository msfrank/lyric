#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <tempo_tracing/error_walker.h>

#include "base_parser_fixture.h"

class ParseConstant : public BaseParserFixture {};

TEST_F(ParseConstant, InvalidHexIntegerIsSyntaxError) {

    auto parseResult = parseModule(R"(
        0xz123
    )");

    ASSERT_TRUE (parseResult.isStatus());

    auto errorWalker = spanset.getErrors();
    ASSERT_EQ (1, errorWalker.numErrors());
    auto span = errorWalker.getError(0);
}
