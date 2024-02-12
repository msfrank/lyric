#ifndef LYRIC_ASSEMBLER_ASSEMBLER_TRACER_H
#define LYRIC_ASSEMBLER_ASSEMBLER_TRACER_H

#include <tempo_tracing/scope_manager.h>

#include "assembler_result.h"

namespace lyric_assembler {

    /**
     * AssemblerTracer encapsulates the logic for logging status conditions and exceptions which are
     * specific to the lyric_assembler namespace.
     */
    class AssemblerTracer {
    public:
        explicit AssemblerTracer(tempo_tracing::ScopeManager *scopeManager);

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
         * @param token
         * @param messageFmt
         * @param args
         */
        template <typename... Args>
        void throwAssemblerInvariant [[noreturn]] (fmt::string_view messageFmt = {}, Args... messageArgs) const
        {
            auto span = m_scopeManager->peekSpan();
            auto status = AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                span->traceId(), span->spanId(), messageFmt, messageArgs...);
            span->logStatus(status, absl::Now(), tempo_tracing::LogSeverity::kError);
            throw AssemblerException(status);
        }
    };
}

#endif // LYRIC_ASSEMBLER_ASSEMBLER_TRACER_H
