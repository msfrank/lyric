#ifndef LYRIC_OBJECT_OBJECT_RESULT_H
#define LYRIC_OBJECT_OBJECT_RESULT_H

#include <lyric_common/symbol_url.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

namespace lyric_object {

    constexpr tempo_utils::SchemaNs kLyricObjectStatusNs("dev.zuri.ns:lyric-object-status-1");

    enum class ObjectCondition {
        kObjectInvariant,
        kMissingSymbol,
    };

    class ObjectStatus : public tempo_utils::TypedStatus<ObjectCondition> {
    public:
        using TypedStatus::TypedStatus;
        static ObjectStatus ok();
        static bool convert(ObjectStatus &dstStatus, const tempo_utils::Status &srcStatus);
    private:
        ObjectStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static ObjectStatus forCondition(
            ObjectCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ObjectStatus(condition, message);
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
        static ObjectStatus forCondition(
            ObjectCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ObjectStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_object::ObjectStatus> {
        using ConditionType = lyric_object::ObjectCondition;
        static bool convert(lyric_object::ObjectStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_object::ObjectStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_object::ObjectCondition> {
        using StatusType = lyric_object::ObjectStatus;
        static constexpr const char *condition_namespace() { return lyric_object::kLyricObjectStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_object::ObjectCondition condition)
        {
            switch (condition) {
                case lyric_object::ObjectCondition::kObjectInvariant:
                    return tempo_utils::StatusCode::kInternal;
                case lyric_object::ObjectCondition::kMissingSymbol:
                    return tempo_utils::StatusCode::kFailedPrecondition;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_object::ObjectCondition condition)
        {
            switch (condition) {
                case lyric_object::ObjectCondition::kObjectInvariant:
                    return "Object invariant";
                case lyric_object::ObjectCondition::kMissingSymbol:
                    return "Missing symbol";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_OBJECT_OBJECT_RESULT_H
