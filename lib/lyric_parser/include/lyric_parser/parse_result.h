#ifndef LYRIC_PARSER_PARSE_RESULT_H
#define LYRIC_PARSER_PARSE_RESULT_H

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_parser {

    constexpr const char *kLyricParserStatusNs = "dev.zuri.ns:lyric-parser-status-1";

    enum class ParseCondition {
        kMissingModule,
        kIncompleteModule,
        kFormattingError,
        kSyntaxError,
        kParseInvariant,
    };

    class ParseStatus : public tempo_utils::TypedStatus<ParseCondition> {
    public:
        using TypedStatus::TypedStatus;
        static ParseStatus ok();
        static bool convert(ParseStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        ParseStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static ParseStatus forCondition(
            ParseCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ParseStatus(condition, message);
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
        static ParseStatus forCondition(
            ParseCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ParseStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_parser::ParseStatus> {
        using ConditionType = lyric_parser::ParseCondition;
        static bool convert(lyric_parser::ParseStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_parser::ParseStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_parser::ParseCondition> {
        using StatusType = lyric_parser::ParseStatus;
        static constexpr const char *condition_namespace() { return lyric_parser::kLyricParserStatusNs; }
        static constexpr StatusCode make_status_code(lyric_parser::ParseCondition condition)
        {
            switch (condition) {
                case lyric_parser::ParseCondition::kMissingModule:
                    return tempo_utils::StatusCode::kFailedPrecondition;
                case lyric_parser::ParseCondition::kIncompleteModule:
                    return tempo_utils::StatusCode::kInvalidArgument;
                case lyric_parser::ParseCondition::kFormattingError:
                    return tempo_utils::StatusCode::kInvalidArgument;
                case lyric_parser::ParseCondition::kSyntaxError:
                    return tempo_utils::StatusCode::kInvalidArgument;
                case lyric_parser::ParseCondition::kParseInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_parser::ParseCondition condition)
        {
            switch (condition) {
                case lyric_parser::ParseCondition::kMissingModule:
                    return "Missing module";
                case lyric_parser::ParseCondition::kIncompleteModule:
                    return "Incomplete module";
                case lyric_parser::ParseCondition::kFormattingError:
                    return "Formatting error";
                case lyric_parser::ParseCondition::kSyntaxError:
                    return "Syntax error";
                case lyric_parser::ParseCondition::kParseInvariant:
                    return "Parse invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_PARSER_PARSE_RESULT_H