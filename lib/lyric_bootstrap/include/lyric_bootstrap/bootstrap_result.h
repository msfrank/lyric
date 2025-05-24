#ifndef LYRIC_BOOTSTRAP_BOOTSTRAP_RESULT_H
#define LYRIC_BOOTSTRAP_BOOTSTRAP_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

namespace lyric_bootstrap {

    constexpr const char *kLyricBootstrapStatusNs = "dev.zuri.ns:lyric-bootstrap-status-1";

    enum class BootstrapCondition {
        kBootstrapInvariant,
    };

    class BootstrapStatus : public tempo_utils::TypedStatus<BootstrapCondition> {
    public:
        using TypedStatus::TypedStatus;
        static BootstrapStatus ok();
        static bool convert(BootstrapStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        BootstrapStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static BootstrapStatus forCondition(
            BootstrapCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return BootstrapStatus(condition, message);
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
        static BootstrapStatus forCondition(
            BootstrapCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return BootstrapStatus(condition, message, traceId, spanId);
        }
    };

    template<typename T>
    class BootstrapResult : public tempo_utils::TypedResult<T,BootstrapStatus> {
    public:
        BootstrapResult() : tempo_utils::TypedResult<T, BootstrapStatus>() {};
        BootstrapResult(const T &result) : tempo_utils::TypedResult<T, BootstrapStatus>(result) {};
        BootstrapResult(const BootstrapStatus &status) : tempo_utils::TypedResult<T, BootstrapStatus>(status) {};
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_bootstrap::BootstrapCondition> {
        using ConditionType = lyric_bootstrap::BootstrapCondition;
        static bool convert(lyric_bootstrap::BootstrapStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_bootstrap::BootstrapStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_bootstrap::BootstrapCondition> {
        using StatusType = lyric_bootstrap::BootstrapStatus;
        static constexpr const char *condition_namespace() { return lyric_bootstrap::kLyricBootstrapStatusNs; }
        static constexpr StatusCode make_status_code(lyric_bootstrap::BootstrapCondition condition)
        {
            switch (condition) {
                case lyric_bootstrap::BootstrapCondition::kBootstrapInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_bootstrap::BootstrapCondition condition)
        {
            switch (condition) {
                case lyric_bootstrap::BootstrapCondition::kBootstrapInvariant:
                    return "Bootstrap invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_BOOTSTRAP_BOOTSTRAP_RESULT_H