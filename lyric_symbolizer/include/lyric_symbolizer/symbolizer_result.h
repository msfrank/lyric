#ifndef LYRIC_SYMBOLIZER_SYMBOLIZER_RESULT_H
#define LYRIC_SYMBOLIZER_SYMBOLIZER_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_symbolizer {

    constexpr tempo_utils::SchemaNs kLyricSymbolizerStatusNs("dev.zuri.ns:lyric-symoblizer-status-1");

    enum class SymbolizerCondition {
        kSyntaxError,
        kSymbolizerInvariant,
    };

    class SymbolizerStatus : public tempo_utils::TypedStatus<SymbolizerCondition> {
    public:
        using TypedStatus::TypedStatus;
        static SymbolizerStatus ok();
        static bool convert(SymbolizerStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        SymbolizerStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static SymbolizerStatus forCondition(
            SymbolizerCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return SymbolizerStatus(condition, message);
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
        static SymbolizerStatus forCondition(
            SymbolizerCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return SymbolizerStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_symbolizer::SymbolizerCondition> {
        using ConditionType = lyric_symbolizer::SymbolizerCondition;
        static bool convert(lyric_symbolizer::SymbolizerStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_symbolizer::SymbolizerStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_symbolizer::SymbolizerCondition> {
        using StatusType = lyric_symbolizer::SymbolizerStatus;
        static constexpr const char *condition_namespace() { return lyric_symbolizer::kLyricSymbolizerStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_symbolizer::SymbolizerCondition condition)
        {
            switch (condition) {
                case lyric_symbolizer::SymbolizerCondition::kSyntaxError:
                case lyric_symbolizer::SymbolizerCondition::kSymbolizerInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_symbolizer::SymbolizerCondition condition)
        {
            switch (condition) {
                case lyric_symbolizer::SymbolizerCondition::kSyntaxError:
                    return "Syntax error";
                case lyric_symbolizer::SymbolizerCondition::kSymbolizerInvariant:
                    return "Symbolizer invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_SYMBOLIZER_SYMBOLIZER_RESULT_H