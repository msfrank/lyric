#ifndef LYRIC_OPTIMIZER_OPTIMIZER_RESULT_H
#define LYRIC_OPTIMIZER_OPTIMIZER_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_optimizer {

    constexpr tempo_utils::SchemaNs kLyricOptimizerStatusNs("dev.zuri.ns:lyric-optimizer-status-1");

    enum class OptimizerCondition {
        kOptimizerInvariant,
    };

    class OptimizerStatus : public tempo_utils::TypedStatus<OptimizerCondition> {
    public:
        using TypedStatus::TypedStatus;
        static OptimizerStatus ok();
        static bool convert(OptimizerStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        OptimizerStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static OptimizerStatus forCondition(
            OptimizerCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return OptimizerStatus(condition, message);
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
        static OptimizerStatus forCondition(
            OptimizerCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return OptimizerStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_optimizer::OptimizerCondition> {
        using ConditionType = lyric_optimizer::OptimizerCondition;
        static bool convert(lyric_optimizer::OptimizerStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_optimizer::OptimizerStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_optimizer::OptimizerCondition> {
        using StatusType = lyric_optimizer::OptimizerStatus;
        static constexpr const char *condition_namespace() { return lyric_optimizer::kLyricOptimizerStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_optimizer::OptimizerCondition condition)
        {
            switch (condition) {
                case lyric_optimizer::OptimizerCondition::kOptimizerInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_optimizer::OptimizerCondition condition)
        {
            switch (condition) {
                case lyric_optimizer::OptimizerCondition::kOptimizerInvariant:
                    return "Optimizer invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_OPTIMIZER_OPTIMIZER_RESULT_H
