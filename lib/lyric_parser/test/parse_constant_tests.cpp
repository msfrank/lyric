#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>
#include <tempo_tracing/error_walker.h>

TEST(ParseConstant, InvalidHexIntegerIsSyntaxError)
{
    lyric_parser::LyricParser parser({});

    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        0xz123
    )", {}, recorder);

//    if (parseResult.isResult()) {
//        auto archetype = parseResult.getResult();
//        TU_CONSOLE_ERR << "unexpected parse success:";
//        TU_CONSOLE_ERR << dump_lyric_archetype(archetype);
//    }
    ASSERT_TRUE (parseResult.isStatus());

    recorder->close();
    auto toSpansetResult = recorder->toSpanset();
    ASSERT_TRUE (toSpansetResult.isResult());
    auto spanset = toSpansetResult.getResult();

    lyric_parser::ParseDiagnosticsOptions options;
    options.sourcePath = "(stdin)";
    auto createDiagnosticsResult = lyric_parser::ParseDiagnostics::create(spanset, options);
    ASSERT_TRUE (createDiagnosticsResult.isResult());
    auto diagnostics = createDiagnosticsResult.getResult();
    diagnostics->printDiagnostics();

    auto errorWalker = spanset.getErrors();
    ASSERT_EQ (1, errorWalker.numErrors());
    auto span = errorWalker.getError(0);
}
