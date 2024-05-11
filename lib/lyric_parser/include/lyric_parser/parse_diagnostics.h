#ifndef LYRIC_PARSER_PARSE_DIAGNOSTICS_H
#define LYRIC_PARSER_PARSE_DIAGNOSTICS_H

#include <tempo_tracing/tempo_spanset.h>

#include "parse_result.h"

namespace lyric_parser {

    struct ParseDiagnosticsOptions {
        std::filesystem::path sourcePath;
    };

    class ParseDiagnostics : public std::enable_shared_from_this<ParseDiagnostics> {

    public:
        static tempo_utils::Result<std::shared_ptr<ParseDiagnostics>> create(
            const tempo_tracing::TempoSpanset &spanset,
            const ParseDiagnosticsOptions &options);

        void printDiagnostics() const;

    private:
        tempo_tracing::TempoSpanset m_spanset;
        ParseDiagnosticsOptions m_options;

        ParseDiagnostics(const tempo_tracing::TempoSpanset &spanset, const ParseDiagnosticsOptions &options);
    };
}

#endif // LYRIC_PARSER_PARSE_DIAGNOSTICS_H