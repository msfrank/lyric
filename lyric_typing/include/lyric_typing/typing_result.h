#ifndef LYRIC_TYPING_TYPING_RESULT_H
#define LYRIC_TYPING_TYPING_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_typing {

    constexpr tempo_utils::SchemaNs kLyricTypingStatusNs("dev.zuri.ns:lyric-typing-status-1");

    enum class TypingCondition {
        kIncompatibleType,
        kMissingType,
        kInvalidType,
        kTypeError,
        kTypingInvariant,
    };

    class TypingStatus : public tempo_utils::TypedStatus<TypingCondition> {
    public:
        using TypedStatus::TypedStatus;
        static TypingStatus ok();
        static bool convert(TypingStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        TypingStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static TypingStatus forCondition(
            TypingCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return TypingStatus(condition, message);
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
        static TypingStatus forCondition(
            TypingCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return TypingStatus(condition, message, traceId, spanId);
        }
    };

    class TypingException : public std::exception {
    public:
        TypingException(const TypingStatus &status) noexcept;
        TypingStatus getStatus() const;
        const char* what() const noexcept override;

    private:
        TypingStatus m_status;
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_typing::TypingCondition> {
        using ConditionType = lyric_typing::TypingCondition;
        static bool convert(lyric_typing::TypingStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_typing::TypingStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_typing::TypingCondition> {
        using StatusType = lyric_typing::TypingStatus;
        static constexpr const char *condition_namespace() { return lyric_typing::kLyricTypingStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_typing::TypingCondition condition)
        {
            switch (condition) {
                case lyric_typing::TypingCondition::kIncompatibleType:
                case lyric_typing::TypingCondition::kMissingType:
                case lyric_typing::TypingCondition::kInvalidType:
                case lyric_typing::TypingCondition::kTypeError:
                case lyric_typing::TypingCondition::kTypingInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_typing::TypingCondition condition)
        {
            switch (condition) {
                case lyric_typing::TypingCondition::kIncompatibleType:
                    return "Incompatible type";
                case lyric_typing::TypingCondition::kMissingType:
                    return "Missing type";
                case lyric_typing::TypingCondition::kInvalidType:
                    return "Invalid type";
                case lyric_typing::TypingCondition::kTypeError:
                    return "Type error";
                case lyric_typing::TypingCondition::kTypingInvariant:
                    return "Typing invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_TYPING_TYPING_RESULT_H