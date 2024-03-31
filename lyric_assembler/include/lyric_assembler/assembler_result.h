#ifndef LYRIC_ASSEMBLER_ASSEMBLER_RESULT_H
#define LYRIC_ASSEMBLER_ASSEMBLER_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_assembler {

    constexpr tempo_utils::SchemaNs kLyricAssemblerStatusNs("dev.zuri.ns:lyric-assembler-status-1");

    enum class AssemblerCondition {
        kTypeError,
        kIncompatibleType,
        kMissingArgument,
        kUnexpectedArgument,
        kMissingType,
        kMissingTemplate,
        kMissingVariable,
        kMissingMember,
        kMissingAction,
        kMissingMethod,
        kMissingImpl,
        kMissingSymbol,
        kModuleNotFound,
        kInvalidLiteral,
        kInvalidSymbol,
        kInvalidBinding,
        kInvalidMutation,
        kInvalidAccess,
        kImplConflict,
        kSymbolAlreadyDefined,
        kImportError,
        kSyntaxError,
        kAssemblerInvariant,
    };

    class AssemblerStatus : public tempo_utils::TypedStatus<AssemblerCondition> {
    public:
        using TypedStatus::TypedStatus;
        static AssemblerStatus ok();
        static bool convert(AssemblerStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        AssemblerStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static AssemblerStatus forCondition(
            AssemblerCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return AssemblerStatus(condition, message);
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
        static AssemblerStatus forCondition(
            AssemblerCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return AssemblerStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_assembler::AssemblerCondition> {
        using ConditionType = lyric_assembler::AssemblerCondition;
        static bool convert(lyric_assembler::AssemblerStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_assembler::AssemblerStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_assembler::AssemblerCondition> {
        using StatusType = lyric_assembler::AssemblerStatus;
        static constexpr const char *condition_namespace() { return lyric_assembler::kLyricAssemblerStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_assembler::AssemblerCondition condition)
        {
            switch (condition) {
                case lyric_assembler::AssemblerCondition::kTypeError:
                case lyric_assembler::AssemblerCondition::kIncompatibleType:
                case lyric_assembler::AssemblerCondition::kMissingArgument:
                case lyric_assembler::AssemblerCondition::kUnexpectedArgument:
                case lyric_assembler::AssemblerCondition::kMissingType:
                case lyric_assembler::AssemblerCondition::kMissingTemplate:
                case lyric_assembler::AssemblerCondition::kMissingVariable:
                case lyric_assembler::AssemblerCondition::kMissingAction:
                case lyric_assembler::AssemblerCondition::kMissingMember:
                case lyric_assembler::AssemblerCondition::kMissingMethod:
                case lyric_assembler::AssemblerCondition::kMissingImpl:
                case lyric_assembler::AssemblerCondition::kMissingSymbol:
                case lyric_assembler::AssemblerCondition::kModuleNotFound:
                case lyric_assembler::AssemblerCondition::kInvalidLiteral:
                case lyric_assembler::AssemblerCondition::kInvalidSymbol:
                case lyric_assembler::AssemblerCondition::kInvalidBinding:
                case lyric_assembler::AssemblerCondition::kInvalidMutation:
                case lyric_assembler::AssemblerCondition::kInvalidAccess:
                case lyric_assembler::AssemblerCondition::kImplConflict:
                case lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined:
                case lyric_assembler::AssemblerCondition::kImportError:
                case lyric_assembler::AssemblerCondition::kSyntaxError:
                case lyric_assembler::AssemblerCondition::kAssemblerInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_assembler::AssemblerCondition condition)
        {
            switch (condition) {
                case lyric_assembler::AssemblerCondition::kTypeError:
                    return "Type error";
                case lyric_assembler::AssemblerCondition::kIncompatibleType:
                    return "Incompatible type";
                case lyric_assembler::AssemblerCondition::kMissingArgument:
                    return "Missing argument";
                case lyric_assembler::AssemblerCondition::kUnexpectedArgument:
                    return "Unexpected argument";
                case lyric_assembler::AssemblerCondition::kMissingType:
                    return "Missing type";
                case lyric_assembler::AssemblerCondition::kMissingTemplate:
                    return "Missing template";
                case lyric_assembler::AssemblerCondition::kMissingVariable:
                    return "Missing variable";
                case lyric_assembler::AssemblerCondition::kMissingAction:
                    return "Missing action";
                case lyric_assembler::AssemblerCondition::kMissingMember:
                    return "Missing member";
                case lyric_assembler::AssemblerCondition::kMissingMethod:
                    return "Missing method";
                case lyric_assembler::AssemblerCondition::kMissingImpl:
                    return "Missing impl";
                case lyric_assembler::AssemblerCondition::kMissingSymbol:
                    return "Missing symbol";
                case lyric_assembler::AssemblerCondition::kModuleNotFound:
                    return "Module not found";
                case lyric_assembler::AssemblerCondition::kInvalidLiteral:
                    return "Invalid literal";
                case lyric_assembler::AssemblerCondition::kInvalidSymbol:
                    return "Invalid symbol";
                case lyric_assembler::AssemblerCondition::kInvalidBinding:
                    return "Invalid binding";
                case lyric_assembler::AssemblerCondition::kInvalidMutation:
                    return "Invalid mutation";
                case lyric_assembler::AssemblerCondition::kInvalidAccess:
                    return "Invalid access";
                case lyric_assembler::AssemblerCondition::kImplConflict:
                    return "Impl conflict";
                case lyric_assembler::AssemblerCondition::kSymbolAlreadyDefined:
                    return "Symbol already defined";
                case lyric_assembler::AssemblerCondition::kImportError:
                    return "Import error";
                case lyric_assembler::AssemblerCondition::kSyntaxError:
                    return "Syntax error";
                case lyric_assembler::AssemblerCondition::kAssemblerInvariant:
                    return "Assembler invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_ASSEMBLER_ASSEMBLER_RESULT_H
