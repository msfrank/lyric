#ifndef LYRIC_TYPING_TYPING_TRACER_H
#define LYRIC_TYPING_TYPING_TRACER_H

#include <lyric_parser/node_walker.h>
#include <lyric_parser/parser_attrs.h>
#include <tempo_tracing/scope_manager.h>

#include "typing_result.h"

namespace lyric_typing {

    /**
     * TypingTracer encapsulates the logic for logging status conditions and exceptions which are
     * specific to the lyric_assembler namespace.
     */
    class TypingTracer {
    public:
        explicit TypingTracer(tempo_tracing::ScopeManager *scopeManager);

    private:
        tempo_tracing::ScopeManager *m_scopeManager;

    public:
        /**
         *
         * @tparam Args
         * @param condition
         * @param severity
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename ConditionType, typename... Args,
            typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        StatusType logAndContinue(
            ConditionType condition,
            tempo_tracing::LogSeverity severity,
            fmt::string_view messageFmt = {},
            Args... messageArgs) const
        {
            auto span = m_scopeManager->peekSpan();
            auto status = StatusType::forCondition(condition, span->traceId(), span->spanId(),
                messageFmt, messageArgs...);
            span->logStatus(status, absl::Now(), severity);
            return status;
        }

        /**
         *
         * @tparam Args
         * @param location
         * @param condition
         * @param severity
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename ConditionType, typename... Args,
            typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        StatusType logAndContinue(
            const lyric_parser::ParseLocation &location,
            ConditionType condition,
            tempo_tracing::LogSeverity severity,
            fmt::string_view messageFmt = {},
            Args... messageArgs) const
        {
            auto span = m_scopeManager->peekSpan();
            auto status = StatusType::forCondition(condition, span->traceId(), span->spanId(),
                messageFmt, messageArgs...);
            auto log = span->logStatus(status, absl::Now(), severity);
            log->putField(lyric_parser::kLyricParserLineNumber, location.lineNumber);
            log->putField(lyric_parser::kLyricParserColumnNumber, location.columnNumber);
            log->putField(lyric_parser::kLyricParserFileOffset, location.fileOffset);
            log->putField(lyric_parser::kLyricParserTextSpan, location.textSpan);
            return status;
        }

        /**
         *
         * @tparam Args
         * @param token
         * @param messageFmt
         * @param args
         */
        template <typename... Args>
        void throwTypingInvariant [[noreturn]] (fmt::string_view messageFmt = {}, Args... messageArgs) const
        {
            auto span = m_scopeManager->peekSpan();
            auto status = TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            throw tempo_utils::StatusException(status);
        }
    };
}

#endif // LYRIC_TYPING_TYPING_TRACER_H
