
#include "base_parser_fixture.h"

#include "lyric_parser/parse_diagnostics.h"

void
BaseParserFixture::SetUp()
{
    recorder = tempo_tracing::TraceRecorder::create();
    lyric_parser::ParserOptions options;
    //options.enableExtraDiagnostics = true;
    //options.reportAllAmbiguities = true;
    parser = std::make_unique<lyric_parser::LyricParser>(options);
}

tempo_utils::Result<lyric_parser::LyricArchetype>
BaseParserFixture::parseModule(std::string_view utf8)
{
    auto sourceUrl = tempo_utils::Url::fromString("/test");
    auto parseResult = parser->parseModule(utf8, sourceUrl, recorder);
    recorder->close();
    TU_ASSIGN_OR_RAISE (spanset, recorder->toSpanset());

    lyric_parser::ParseDiagnosticsOptions options;
    options.sourcePath = "(stdin)";
    TU_ASSIGN_OR_RAISE (diagnostics, lyric_parser::ParseDiagnostics::create(spanset, options));

    return parseResult;
}