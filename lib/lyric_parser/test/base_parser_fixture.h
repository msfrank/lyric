#ifndef BASE_PARSER_FIXTURE_H
#define BASE_PARSER_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/parse_diagnostics.h>

class BaseParserFixture : public ::testing::Test {
protected:
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder;
    lyric_parser::ParserOptions parserOptions;
    lyric_parser::ParseDiagnosticsOptions parseDiagnosticsOptions;
    tempo_tracing::TempoSpanset spanset;
    std::shared_ptr<lyric_parser::ParseDiagnostics> diagnostics;

    void SetUp() override;

    lyric_parser::LyricParser *getParser();

    tempo_utils::Result<lyric_parser::LyricArchetype> parseModule(std::string_view utf8);

private:
    std::unique_ptr<lyric_parser::LyricParser> m_parser;
};

#endif //BASE_PARSER_FIXTURE_H
