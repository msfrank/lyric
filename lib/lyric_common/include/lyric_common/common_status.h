#ifndef LYRIC_COMMON_COMMON_STATUS_H
#define LYRIC_COMMON_COMMON_STATUS_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/status.h>

namespace lyric_common {

    constexpr const char *kLyricCommonStatusNs = "dev.zuri.ns:lyric-common-status-1";

    enum class CommonCondition {
        kCommonInvariant,
    };

    class CommonStatus : public tempo_utils::TypedStatus<CommonCondition> {
    public:
        using TypedStatus::TypedStatus;
        static bool convert(CommonStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        CommonStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @param condition
         * @param message
         * @return
         */
        static CommonStatus forCondition(
            CommonCondition condition,
            std::string_view message)
        {
            return CommonStatus(condition, message);
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
        static CommonStatus forCondition(
            CommonCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return CommonStatus(condition, message);
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
        static CommonStatus forCondition(
            CommonCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return CommonStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_common::CommonCondition> {
        using ConditionType = lyric_common::CommonCondition;
        static bool convert(lyric_common::CommonStatus &dstStatus, const Status &srcStatus)
        {
            return lyric_common::CommonStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_common::CommonCondition> {
        using StatusType = lyric_common::CommonStatus;
        static constexpr const char *condition_namespace() { return lyric_common::kLyricCommonStatusNs; }
        static constexpr StatusCode make_status_code(lyric_common::CommonCondition condition)
        {
            switch (condition) {
                case lyric_common::CommonCondition::kCommonInvariant:
                    return StatusCode::kInternal;
                default:
                    return StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_common::CommonCondition condition)
        {
            switch (condition) {
                case lyric_common::CommonCondition::kCommonInvariant:
                    return "Common invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_COMMON_COMMON_STATUS_H