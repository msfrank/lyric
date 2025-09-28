#ifndef LYRIC_REWRITER_REWRITER_RESULT_H
#define LYRIC_REWRITER_REWRITER_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_rewriter {

    constexpr const char *kLyricRewriterStatusNs = "dev.zuri.ns:lyric-rewriter-status-1";

    enum class RewriterCondition {
        kSyntaxError,
        kRewriterInvariant,
    };

    class RewriterStatus : public tempo_utils::TypedStatus<RewriterCondition> {
    public:
        using TypedStatus::TypedStatus;
        static bool convert(RewriterStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        RewriterStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static RewriterStatus forCondition(
            RewriterCondition condition,
            std::string_view message)
        {
            return RewriterStatus(condition, message);
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
        static RewriterStatus forCondition(
            RewriterCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return RewriterStatus(condition, message);
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
        static RewriterStatus forCondition(
            RewriterCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return RewriterStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_rewriter::RewriterCondition> {
        using ConditionType = lyric_rewriter::RewriterCondition;
        static bool convert(lyric_rewriter::RewriterStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_rewriter::RewriterStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_rewriter::RewriterCondition> {
        using StatusType = lyric_rewriter::RewriterStatus;
        static constexpr const char *condition_namespace() { return lyric_rewriter::kLyricRewriterStatusNs; }
        static constexpr StatusCode make_status_code(lyric_rewriter::RewriterCondition condition)
        {
            switch (condition) {
                case lyric_rewriter::RewriterCondition::kSyntaxError:
                case lyric_rewriter::RewriterCondition::kRewriterInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_rewriter::RewriterCondition condition)
        {
            switch (condition) {
                case lyric_rewriter::RewriterCondition::kSyntaxError:
                    return "Syntax error";
                case lyric_rewriter::RewriterCondition::kRewriterInvariant:
                    return "Rewriter invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_REWRITER_REWRITER_RESULT_H