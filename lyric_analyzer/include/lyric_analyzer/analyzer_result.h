#ifndef LYRIC_ANALYZER_ANALYZER_RESULT_H
#define LYRIC_ANALYZER_ANALYZER_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_analyzer {

    constexpr tempo_utils::SchemaNs kLyricAnalyzerStatusNs("dev.zuri.ns:lyric-analyzer-status-1");

    enum class AnalyzerCondition {
        kSyntaxError,
        kAnalyzerInvariant,
    };

    class AnalyzerStatus : public tempo_utils::TypedStatus<AnalyzerCondition> {
    public:
        using TypedStatus::TypedStatus;
        static AnalyzerStatus ok();
        static bool convert(AnalyzerStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        AnalyzerStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @tparam Args
         * @param condition
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename... Args>
        static AnalyzerStatus forCondition(
            AnalyzerCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return AnalyzerStatus(condition, message);
        }
        /**
         *
         * @tparam Args
         * @param condition
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename... Args>
        static AnalyzerStatus forCondition(
            AnalyzerCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return AnalyzerStatus(condition, message, traceId, spanId);
        }
    };

    class AnalyzerException : public std::exception {
    public:
        AnalyzerException(const AnalyzerStatus &status) noexcept;
        AnalyzerStatus getStatus() const;
        const char* what() const noexcept override;

    private:
        AnalyzerStatus m_status;
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_analyzer::AnalyzerCondition> {
        using ConditionType = lyric_analyzer::AnalyzerCondition;
        static bool convert(lyric_analyzer::AnalyzerStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_analyzer::AnalyzerStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_analyzer::AnalyzerCondition> {
        using StatusType = lyric_analyzer::AnalyzerStatus;
        static constexpr const char *condition_namespace() { return lyric_analyzer::kLyricAnalyzerStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_analyzer::AnalyzerCondition condition)
        {
            switch (condition) {
                case lyric_analyzer::AnalyzerCondition::kSyntaxError:
                case lyric_analyzer::AnalyzerCondition::kAnalyzerInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_analyzer::AnalyzerCondition condition)
        {
            switch (condition) {
                case lyric_analyzer::AnalyzerCondition::kSyntaxError:
                    return "Syntax error";
                case lyric_analyzer::AnalyzerCondition::kAnalyzerInvariant:
                    return "Analyzer invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_ANALYZER_ANALYZER_RESULT_H