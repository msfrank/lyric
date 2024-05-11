//#ifndef LYRIC_BUILD_METADATA_RESULT_H
//#define LYRIC_BUILD_METADATA_RESULT_H
//
//#include <string>
//
//#include <fmt/core.h>
//#include <fmt/format.h>
//
//#include <tempo_utils/result.h>
//#include <tempo_utils/status.h>
//
//namespace lyric_build {
//
//    constexpr tempo_utils::SchemaNs kLyricBuildMetadataStatusNs("dev.zuri.ns:lyric-build-metadata-status-1");
//
//    enum class MetadataCondition {
//        kNotFound,
//        kConflict,
//        kWrongType,
//    };
//
//    class MetadataStatus : public tempo_utils::TypedStatus<MetadataCondition> {
//    public:
//        using TypedStatus::TypedStatus;
//
//        static MetadataStatus ok();
//
//        static bool convert(MetadataStatus &dstStatus, const tempo_utils::Status &srcStatus);
//
//    private:
//        MetadataStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);
//
//    public:
//        /**
//         *
//         * @tparam Args
//         * @param condition
//         * @param messageFmt
//         * @param messageArgs
//         * @return
//         */
//        template <typename... Args>
//        static MetadataStatus forCondition(
//            MetadataCondition condition,
//            fmt::string_view messageFmt = {},
//            Args... messageArgs)
//        {
//            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
//            return MetadataStatus(condition, message);
//        }
//        /**
//         *
//         * @tparam Args
//         * @param condition
//         * @param messageFmt
//         * @param messageArgs
//         * @return
//         */
//        template <typename... Args>
//        static MetadataStatus forCondition(
//            MetadataCondition condition,
//            tempo_utils::TraceId traceId,
//            tempo_utils::SpanId spanId,
//            fmt::string_view messageFmt = {},
//            Args... messageArgs)
//        {
//            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
//            return MetadataStatus(condition, message, traceId, spanId);
//        }
//    };
//
//    template<typename T>
//    class MetadataResult : public tempo_utils::TypedResult<T,MetadataStatus> {
//    public:
//        MetadataResult() : tempo_utils::TypedResult<T, MetadataStatus>() {};
//        MetadataResult(const T &result) : tempo_utils::TypedResult<T, MetadataStatus>(result) {};
//        MetadataResult(const MetadataStatus &status) : tempo_utils::TypedResult<T, MetadataStatus>(status) {};
//    };
//
//    class MetadataException : public std::exception {
//    public:
//        MetadataException(const MetadataStatus &status) noexcept;
//        MetadataStatus getStatus() const;
//        const char* what() const noexcept override;
//
//    private:
//        MetadataStatus m_status;
//    };
//}
//
//namespace tempo_utils {
//
//    template<>
//    struct StatusTraits<lyric_build::MetadataCondition> {
//        using ConditionType = lyric_build::MetadataCondition;
//        static bool convert(lyric_build::MetadataStatus &dstStatus, const tempo_utils::Status &srcStatus)
//        {
//            return lyric_build::MetadataStatus::convert(dstStatus, srcStatus);
//        }
//    };
//
//    template<>
//    struct ConditionTraits<lyric_build::MetadataCondition> {
//        using StatusType = lyric_build::MetadataStatus;
//        static constexpr const char *condition_namespace() { return lyric_build::kLyricBuildMetadataStatusNs.getNs(); }
//        static constexpr StatusCode make_status_code(lyric_build::MetadataCondition condition)
//        {
//            switch (condition) {
//                case lyric_build::MetadataCondition::kConflict:
//                case lyric_build::MetadataCondition::kNotFound:
//                case lyric_build::MetadataCondition::kWrongType:
//                    return StatusCode::kInternal;
//                default:
//                    return tempo_utils::StatusCode::kUnknown;
//            }
//        };
//        static constexpr const char *make_error_message(lyric_build::MetadataCondition condition)
//        {
//            switch (condition) {
//                case lyric_build::MetadataCondition::kConflict:
//                    return "Conflict";
//                case lyric_build::MetadataCondition::kNotFound:
//                    return "Not found";
//                case lyric_build::MetadataCondition::kWrongType:
//                    return "Wrong type";
//                default:
//                    return "INVALID";
//            }
//        }
//    };
//}
//
//#endif // LYRIC_BUILD_METADATA_RESULT_H
