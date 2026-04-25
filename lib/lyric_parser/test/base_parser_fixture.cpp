
#include "base_parser_fixture.h"

#include "lyric_parser/parse_diagnostics.h"

void
BaseParserFixture::SetUp()
{
    tempo_utils::LoggingConfiguration loggingConf;
    loggingConf.severityFilter = tempo_utils::SeverityFilter::kVeryVerbose;
    tempo_utils::init_logging(loggingConf);
    recorder = tempo_tracing::TraceRecorder::create();
    parseDiagnosticsOptions.sourcePath = "(stdin)";
}

lyric_parser::LyricParser *
BaseParserFixture::getParser()
{
    if (m_parser == nullptr) {
        m_parser = std::make_unique<lyric_parser::LyricParser>(parserOptions);
    }
    return m_parser.get();
}

tempo_utils::Result<lyric_parser::LyricArchetype>
BaseParserFixture::parseModule(std::string_view utf8)
{
    auto *parser = getParser();
    auto sourceUrl = tempo_utils::Url::fromString("/test");
    auto parseResult = parser->parseModule(utf8, sourceUrl, recorder);
    recorder->close();

    TU_ASSIGN_OR_RAISE (spanset, recorder->toSpanset());
    TU_ASSIGN_OR_RAISE (diagnostics, lyric_parser::ParseDiagnostics::create(spanset, parseDiagnosticsOptions));

    return parseResult;
}