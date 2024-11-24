#ifndef LYRIC_ARCHIVER_ARCHIVER_RESULT_H
#define LYRIC_ARCHIVER_ARCHIVER_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_archiver {

    constexpr tempo_utils::SchemaNs kLyricArchiverStatusNs("dev.zuri.ns:lyric-archiver-status-1");

    enum class ArchiverCondition {
        kArchiverInvariant,
    };

    class ArchiverStatus : public tempo_utils::TypedStatus<ArchiverCondition> {
    public:
        using TypedStatus::TypedStatus;
        static ArchiverStatus ok();
        static bool convert(ArchiverStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        ArchiverStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static ArchiverStatus forCondition(
            ArchiverCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ArchiverStatus(condition, message);
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
        static ArchiverStatus forCondition(
            ArchiverCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ArchiverStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_archiver::ArchiverCondition> {
        using ConditionType = lyric_archiver::ArchiverCondition;
        static bool convert(lyric_archiver::ArchiverStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_archiver::ArchiverStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_archiver::ArchiverCondition> {
        using StatusType = lyric_archiver::ArchiverStatus;
        static constexpr const char *condition_namespace() { return lyric_archiver::kLyricArchiverStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_archiver::ArchiverCondition condition)
        {
            switch (condition) {
                case lyric_archiver::ArchiverCondition::kArchiverInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_archiver::ArchiverCondition condition)
        {
            switch (condition) {
                case lyric_archiver::ArchiverCondition::kArchiverInvariant:
                    return "Archiver invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_ARCHIVER_ARCHIVER_RESULT_H
