#ifndef LYRIC_PACKAGING_PACKAGE_RESULT_H
#define LYRIC_PACKAGING_PACKAGE_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_packaging {

    constexpr tempo_utils::SchemaNs kLyricPackagingStatusNs("dev.zuri.ns:lyric-packaging-status-1");

    enum class PackageCondition {
        kInvalidHeader,
        kInvalidManifest,
        kDuplicateEntry,
        kDuplicateAttr,
        kDuplicateNamespace,
        kPackageInvariant,
    };

    class PackageStatus : public tempo_utils::TypedStatus<PackageCondition> {
    public:
        using TypedStatus::TypedStatus;
        static PackageStatus ok();
        static bool convert(PackageStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        PackageStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static PackageStatus forCondition(
            PackageCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return PackageStatus(condition, message);
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
        static PackageStatus forCondition(
            PackageCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return PackageStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_packaging::PackageCondition> {
        using ConditionType = lyric_packaging::PackageCondition;
        static bool convert(lyric_packaging::PackageStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_packaging::PackageStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_packaging::PackageCondition> {
        using StatusType = lyric_packaging::PackageStatus;
        static constexpr const char *condition_namespace() { return lyric_packaging::kLyricPackagingStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_packaging::PackageCondition condition)
        {
            switch (condition) {
                case lyric_packaging::PackageCondition::kInvalidHeader:
                    return StatusCode::kInvalidArgument;
                case lyric_packaging::PackageCondition::kInvalidManifest:
                    return StatusCode::kInvalidArgument;
                case lyric_packaging::PackageCondition::kDuplicateEntry:
                    return StatusCode::kInvalidArgument;
                case lyric_packaging::PackageCondition::kDuplicateAttr:
                    return StatusCode::kInvalidArgument;
                case lyric_packaging::PackageCondition::kDuplicateNamespace:
                    return StatusCode::kInvalidArgument;
                case lyric_packaging::PackageCondition::kPackageInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_packaging::PackageCondition condition)
        {
            switch (condition) {
                case lyric_packaging::PackageCondition::kInvalidHeader:
                    return "Invalid header";
                case lyric_packaging::PackageCondition::kInvalidManifest:
                    return "Invalid manifest";
                case lyric_packaging::PackageCondition::kDuplicateEntry:
                    return "Duplicate entry";
                case lyric_packaging::PackageCondition::kDuplicateAttr:
                    return "Duplicate attr";
                case lyric_packaging::PackageCondition::kDuplicateNamespace:
                    return "Duplicate namespace";
                case lyric_packaging::PackageCondition::kPackageInvariant:
                    return "Package invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_PACKAGING_PACKAGE_RESULT_H