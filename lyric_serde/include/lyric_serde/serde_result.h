#ifndef LYRIC_SERDE_SERDE_RESULT_H
#define LYRIC_SERDE_SERDE_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_serde {

    constexpr tempo_utils::SchemaNs kLyricSerdeStatusNs("dev.zuri.ns:lyric-serde-status-1");

    enum class SerdeCondition {
        kInvalidPatchset,
        kDuplicateNamespace,
        kDuplicateChange,
        kSerdeInvariant,
    };

    class SerdeStatus : public tempo_utils::TypedStatus<SerdeCondition> {
    public:
        using TypedStatus::TypedStatus;
        static SerdeStatus ok();
        static bool convert(SerdeStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        SerdeStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static SerdeStatus forCondition(
            SerdeCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return SerdeStatus(condition, message);
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
        static SerdeStatus forCondition(
            SerdeCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return SerdeStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_serde::SerdeCondition> {
        using ConditionType = lyric_serde::SerdeCondition;
        static bool convert(lyric_serde::SerdeStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_serde::SerdeStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_serde::SerdeCondition> {
        using StatusType = lyric_serde::SerdeStatus;
        static constexpr const char *condition_namespace() { return lyric_serde::kLyricSerdeStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_serde::SerdeCondition condition)
        {
            switch (condition) {
                case lyric_serde::SerdeCondition::kInvalidPatchset:
                    return StatusCode::kInvalidArgument;
                case lyric_serde::SerdeCondition::kDuplicateNamespace:
                    return StatusCode::kInvalidArgument;
                case lyric_serde::SerdeCondition::kDuplicateChange:
                    return StatusCode::kInvalidArgument;
                case lyric_serde::SerdeCondition::kSerdeInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_serde::SerdeCondition condition)
        {
            switch (condition) {
                case lyric_serde::SerdeCondition::kInvalidPatchset:
                    return "Invalid patchset";
                case lyric_serde::SerdeCondition::kDuplicateNamespace:
                    return "Duplicate namespace";
                case lyric_serde::SerdeCondition::kDuplicateChange:
                    return "Duplicate change";
                case lyric_serde::SerdeCondition::kSerdeInvariant:
                    return "Serde invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_SERDE_SERDE_RESULT_H