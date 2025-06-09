
#include <lyric_parser/parse_diagnostics.h>
#include <lyric_parser/parser_attrs.h>
#include <tempo_tracing/error_walker.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/log_helpers.h>

lyric_parser::ParseDiagnostics::ParseDiagnostics(
    const tempo_tracing::TempoSpanset &spanset,
    const ParseDiagnosticsOptions &options)
    : m_spanset(spanset),
      m_options(options)
{
    TU_ASSERT (m_spanset.isValid());
}

tempo_utils::Result<std::shared_ptr<lyric_parser::ParseDiagnostics>>
lyric_parser::ParseDiagnostics::create(
    const tempo_tracing::TempoSpanset &spanset,
    const ParseDiagnosticsOptions &options)
{
    if (!spanset.isValid())
        return ParseStatus::forCondition(
            ParseCondition::kParseInvariant, "invalid spanset");
    return std::shared_ptr<ParseDiagnostics>(new ParseDiagnostics(spanset, options));
}

static tempo_utils::Result<std::pair<tu_int64,tu_int64>>
parse_position(const tempo_tracing::SpanWalker &spanWalker)
{
    TU_ASSERT (spanWalker.isValid());

    // if either line or column number attrs are not present, then return invalid position
    if (!spanWalker.hasTag(lyric_parser::kLyricParserLineNumber)
        || !spanWalker.hasTag(lyric_parser::kLyricParserColumnNumber))
        return std::pair<tu_int64, tu_int64>{-1,-1};

    tempo_utils::Status status;

    tu_int64 lineNumber;
    status = spanWalker.parseTag(lyric_parser::kLyricParserLineNumber, lineNumber);
    if (status.notOk())
        return status;

    tu_int64 columnNumber;
    status = spanWalker.parseTag(lyric_parser::kLyricParserColumnNumber, columnNumber);
    if (status.notOk())
        return status;

    return std::pair<tu_int64, tu_int64>{lineNumber, columnNumber};
}

static tempo_utils::Status
print_symbol_context(
    const tempo_tracing::SpanWalker &spanWalker,
    int indent,
    const lyric_parser::ParseDiagnosticsOptions &options)
{
    if (!spanWalker.isValid())
        return {};

    if (spanWalker.hasParent())
        print_symbol_context(spanWalker.getParent(), indent, options);

    if (spanWalker.hasTag(lyric_parser::kLyricParserIdentifier))
        return {};

    std::string identifier;
    auto status = spanWalker.parseTag(lyric_parser::kLyricParserIdentifier, identifier);
    if (status.notOk())
        return status;

    TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "...in definition " << identifier;
    return {};
}

static tempo_utils::Status
print_position_context(
    const tempo_tracing::SpanWalker &spanWalker,
    int indent,
    const lyric_parser::ParseDiagnosticsOptions &options)
{
    // if walker is invalid, then don't print context
    if (!spanWalker.isValid())
        return {};

    auto parsePositionResult = parse_position(spanWalker);
    if (parsePositionResult.isStatus())
        return parsePositionResult.getStatus();
    auto position = parsePositionResult.getResult();
    auto lineNumber = position.first;
    auto columnNumber = position.second;

    if (lineNumber >= 0 && columnNumber >= 0) {
        TU_CONSOLE_OUT << tempo_utils::Indent(indent)
            << "...at " << options.sourcePath.string()
            << ":" << lineNumber << ":" << columnNumber;
    }

    return {};
}

static tempo_utils::Status
print_error(
    const tempo_tracing::SpanWalker &spanWalker,
    int indent,
    const lyric_parser::ParseDiagnosticsOptions &options)
{
    TU_ASSERT (spanWalker.isValid());

    tempo_utils::Status status;

    // print symbol context
    status = print_symbol_context(spanWalker.getParent(), indent, options);
    if (status.notOk())
        return status;

    // print position context
    status = print_position_context(spanWalker, indent, options);
    if (status.notOk())
        return status;

    // print message
    std::string message;
    status = spanWalker.parseTag(tempo_tracing::kOpentracingMessage, message);
    if (status.notOk())
        return status;

    TU_CONSOLE_OUT << tempo_utils::Indent(indent) << "ERROR: " << message;

    for (int i = 0; i < spanWalker.numLogs(); i++) {
        auto logWalker = spanWalker.getLog(i);
        status = logWalker.parseField(tempo_tracing::kOpentracingMessage, message);
        if (status.notOk())
            return status;
        TU_CONSOLE_OUT << tempo_utils::Indent(indent + 2) << message;
    }

    return {};
}

void
lyric_parser::ParseDiagnostics::printDiagnostics() const
{
    auto errorWalker = m_spanset.getErrors();

    for (int i = 0; i < errorWalker.numErrors(); i++) {
        auto error = errorWalker.getError(i);
        print_error(error, 0, m_options);
    }
}
