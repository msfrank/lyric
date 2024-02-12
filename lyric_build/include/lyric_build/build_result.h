#ifndef LYRIC_BUILD_BUILD_RESULT_H
#define LYRIC_BUILD_BUILD_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/status.h>

namespace lyric_build {

    constexpr tempo_utils::SchemaNs kLyricBuildStatusNs("dev.zuri.ns:lyric-build-status-1");

    enum class BuildCondition {
        kArtifactNotFound,
        kDependencyFailure,
        kInvalidConfiguration,
        kHashMismatch,
        kMissingHash,
        kTaskExists,
        kTaskIncomplete,
        kTaskFailure,
        kInstallError,
        kBuildInvariant,
    };

    class BuildStatus : public tempo_utils::TypedStatus<BuildCondition> {
    public:
        using TypedStatus::TypedStatus;
        static BuildStatus ok();
        static bool convert(BuildStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        BuildStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static BuildStatus forCondition(
            BuildCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return BuildStatus(condition, message);
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
        static BuildStatus forCondition(
            BuildCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return BuildStatus(condition, message, traceId, spanId);
        }
    };

    class BuildException : public std::exception {
    public:
        BuildException(const BuildStatus &status) noexcept;
        BuildStatus getStatus() const;
        const char* what() const noexcept override;

    private:
        BuildStatus m_status;
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_build::BuildStatus> {
        using ConditionType = lyric_build::BuildCondition;
        static bool convert(lyric_build::BuildStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_build::BuildStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_build::BuildCondition> {
        using StatusType = lyric_build::BuildStatus;
        static constexpr const char *condition_namespace() { return lyric_build::kLyricBuildStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_build::BuildCondition condition)
        {
            switch (condition) {
                case lyric_build::BuildCondition::kArtifactNotFound:
                case lyric_build::BuildCondition::kDependencyFailure:
                case lyric_build::BuildCondition::kInvalidConfiguration:
                case lyric_build::BuildCondition::kHashMismatch:
                case lyric_build::BuildCondition::kMissingHash:
                case lyric_build::BuildCondition::kTaskExists:
                case lyric_build::BuildCondition::kTaskIncomplete:
                case lyric_build::BuildCondition::kTaskFailure:
                case lyric_build::BuildCondition::kInstallError:
                case lyric_build::BuildCondition::kBuildInvariant:
                    return StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_build::BuildCondition condition)
        {
            switch (condition) {
                case lyric_build::BuildCondition::kArtifactNotFound:
                    return "Artifact not found";
                case lyric_build::BuildCondition::kDependencyFailure:
                    return "Dependency failure";
                case lyric_build::BuildCondition::kInvalidConfiguration:
                    return "Invalid configuration";
                case lyric_build::BuildCondition::kHashMismatch:
                    return "Hash mismatch";
                case lyric_build::BuildCondition::kMissingHash:
                    return "Missing hash";
                case lyric_build::BuildCondition::kTaskExists:
                    return "Task exists";
                case lyric_build::BuildCondition::kTaskIncomplete:
                    return "Task incomplete";
                case lyric_build::BuildCondition::kTaskFailure:
                    return "Task failure";
                case lyric_build::BuildCondition::kInstallError:
                    return "Install error";
                case lyric_build::BuildCondition::kBuildInvariant:
                    return "Build invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_BUILD_BUILD_RESULT_H