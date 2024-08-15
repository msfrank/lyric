#ifndef LYRIC_SYMBOLIZER_SYMBOLIZE_HANDLE_H
#define LYRIC_SYMBOLIZER_SYMBOLIZE_HANDLE_H

#include <lyric_assembler/object_state.h>
#include <lyric_common/symbol_url.h>
#include <lyric_parser/node_walker.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_symbolizer/symbolizer_result.h>
#include <tempo_tracing/span_log.h>

namespace lyric_symbolizer::internal {

    class SymbolizeHandle {

    public:
        explicit SymbolizeHandle(lyric_assembler::ObjectState *state);
        SymbolizeHandle(const lyric_common::SymbolUrl &blockUrl, SymbolizeHandle *parent);

        SymbolizeHandle *blockParent() const;
        lyric_assembler::ObjectState *blockState() const;
        std::shared_ptr<tempo_tracing::TraceSpan> getSpan() const;

        tempo_utils::Result<lyric_common::SymbolUrl>
        declareSymbol(const std::string &identifier, lyric_object::LinkageSection section);

        tempo_utils::Status declareImport(const lyric_common::ModuleLocation &location);

    private:
        lyric_common::SymbolUrl m_blockUrl;
        SymbolizeHandle *m_parent;
        lyric_assembler::ObjectState *m_state;
        std::shared_ptr<tempo_tracing::TraceSpan> m_span;

    public:
        /**
          *
          * @tparam Args
          * @param walker
          * @param messageFmt
          * @param args
          */
        template <typename... Args>
        void throwSyntaxError [[noreturn]] (
            const lyric_parser::NodeWalker &walker,
            fmt::string_view messageFmt = {},
            Args... messageArgs) const
        {
            auto *scopeManager = m_state->scopeManager();
            auto span = scopeManager->peekSpan();
            auto status = SymbolizerStatus::forCondition(
                SymbolizerCondition::kSyntaxError,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(lyric_parser::kLyricParserIdentifier, m_blockUrl.getSymbolPath().toString());
            log->putField(lyric_parser::kLyricParserLineNumber, static_cast<tu_int64>(walker.getLineNumber()));
            log->putField(lyric_parser::kLyricParserColumnNumber, static_cast<tu_int64>(walker.getColumnNumber()));
            log->putField(lyric_parser::kLyricParserFileOffset, static_cast<tu_int64>(walker.getFileOffset()));
            log->putField(lyric_parser::kLyricParserTextSpan, static_cast<tu_int64>(walker.getTextSpan()));
            throw tempo_utils::StatusException(status);
        };

        /**
          *
          * @tparam Args
          * @param walker
          * @param messageFmt
          * @param args
          */
        template <typename... Args>
        void throwSymbolizerInvariant [[noreturn]] (
            const lyric_parser::NodeWalker &walker,
            fmt::string_view messageFmt = {},
            Args... messageArgs) const
        {
            auto *scopeManager = m_state->scopeManager();
            auto span = scopeManager->peekSpan();
            auto status = SymbolizerStatus::forCondition(
                SymbolizerCondition::kSymbolizerInvariant,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            log->putField(lyric_parser::kLyricParserIdentifier, m_blockUrl.getSymbolPath().toString());
            log->putField(lyric_parser::kLyricParserLineNumber, static_cast<tu_int64>(walker.getLineNumber()));
            log->putField(lyric_parser::kLyricParserColumnNumber, static_cast<tu_int64>(walker.getColumnNumber()));
            log->putField(lyric_parser::kLyricParserFileOffset, static_cast<tu_int64>(walker.getFileOffset()));
            log->putField(lyric_parser::kLyricParserTextSpan, static_cast<tu_int64>(walker.getTextSpan()));
            throw tempo_utils::StatusException(status);
        };
    };
}

#endif // LYRIC_SYMBOLIZER_SYMBOLIZE_HANDLE_H
