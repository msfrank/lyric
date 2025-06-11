#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <tempo_test/result_matchers.h>

#include "base_parser_fixture.h"

class ParseDef : public BaseParserFixture {};

TEST_F(ParseDef, NoArgumentsWithReturnType) {

    auto parseResult = parseModule(R"(
        def NoArguments(): Int {
            42
        }
    )");

    ASSERT_THAT (parseResult, tempo_test::IsResult());
}

TEST_F(ParseDef, ErrorExtraClosingCurly) {

    auto parseResult = parseModule(R"(
        def NoArguments(): Int {
            42
        }}
    )");

    ASSERT_THAT (parseResult, tempo_test::IsStatus());
    auto statusMessage = parseResult.getStatus().getMessage();
    ASSERT_THAT (statusMessage, ::testing::ContainsRegex("Extra '}' closing def"));
}
