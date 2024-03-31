#ifndef LYRIC_COMPILER_COMPILER_RESULT_H
#define LYRIC_COMPILER_COMPILER_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/status.h>

namespace lyric_compiler {

    constexpr tempo_utils::SchemaNs kLyricCompilerStatusNs("dev.zuri.ns:lyric-compiler-status-1");

    enum class CompilerCondition {
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
        kCompilerInvariant,
    };

    class CompilerStatus : public tempo_utils::TypedStatus<CompilerCondition> {
    public:
        using TypedStatus::TypedStatus;
        static CompilerStatus ok();
        static bool convert(CompilerStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        CompilerStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static CompilerStatus forCondition(
            CompilerCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return CompilerStatus(condition, message);
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
        static CompilerStatus forCondition(
            CompilerCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return CompilerStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_compiler::CompilerCondition> {
        using ConditionType = lyric_compiler::CompilerCondition;
        static bool convert(lyric_compiler::CompilerStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_compiler::CompilerStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_compiler::CompilerCondition> {
        using StatusType = lyric_compiler::CompilerStatus;
        static constexpr const char *condition_namespace() { return lyric_compiler::kLyricCompilerStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_compiler::CompilerCondition condition)
        {
            switch (condition) {
                case lyric_compiler::CompilerCondition::kTypeError:
                case lyric_compiler::CompilerCondition::kIncompatibleType:
                case lyric_compiler::CompilerCondition::kMissingArgument:
                case lyric_compiler::CompilerCondition::kUnexpectedArgument:
                case lyric_compiler::CompilerCondition::kMissingType:
                case lyric_compiler::CompilerCondition::kMissingTemplate:
                case lyric_compiler::CompilerCondition::kMissingVariable:
                case lyric_compiler::CompilerCondition::kMissingAction:
                case lyric_compiler::CompilerCondition::kMissingMember:
                case lyric_compiler::CompilerCondition::kMissingMethod:
                case lyric_compiler::CompilerCondition::kMissingImpl:
                case lyric_compiler::CompilerCondition::kMissingSymbol:
                case lyric_compiler::CompilerCondition::kModuleNotFound:
                case lyric_compiler::CompilerCondition::kInvalidLiteral:
                case lyric_compiler::CompilerCondition::kInvalidSymbol:
                case lyric_compiler::CompilerCondition::kInvalidBinding:
                case lyric_compiler::CompilerCondition::kInvalidMutation:
                case lyric_compiler::CompilerCondition::kInvalidAccess:
                case lyric_compiler::CompilerCondition::kImplConflict:
                case lyric_compiler::CompilerCondition::kSymbolAlreadyDefined:
                case lyric_compiler::CompilerCondition::kImportError:
                case lyric_compiler::CompilerCondition::kSyntaxError:
                case lyric_compiler::CompilerCondition::kCompilerInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_compiler::CompilerCondition condition)
        {
            switch (condition) {
                case lyric_compiler::CompilerCondition::kTypeError:
                    return "Type error";
                case lyric_compiler::CompilerCondition::kIncompatibleType:
                    return "Incompatible type";
                case lyric_compiler::CompilerCondition::kMissingArgument:
                    return "Missing argument";
                case lyric_compiler::CompilerCondition::kUnexpectedArgument:
                    return "Unexpected argument";
                case lyric_compiler::CompilerCondition::kMissingType:
                    return "Missing type";
                case lyric_compiler::CompilerCondition::kMissingTemplate:
                    return "Missing template";
                case lyric_compiler::CompilerCondition::kMissingVariable:
                    return "Missing variable";
                case lyric_compiler::CompilerCondition::kMissingAction:
                    return "Missing action";
                case lyric_compiler::CompilerCondition::kMissingMember:
                    return "Missing member";
                case lyric_compiler::CompilerCondition::kMissingMethod:
                    return "Missing method";
                case lyric_compiler::CompilerCondition::kMissingImpl:
                    return "Missing impl";
                case lyric_compiler::CompilerCondition::kMissingSymbol:
                    return "Missing symbol";
                case lyric_compiler::CompilerCondition::kModuleNotFound:
                    return "Module not found";
                case lyric_compiler::CompilerCondition::kInvalidLiteral:
                    return "Invalid literal";
                case lyric_compiler::CompilerCondition::kInvalidSymbol:
                    return "Invalid symbol";
                case lyric_compiler::CompilerCondition::kInvalidBinding:
                    return "Invalid binding";
                case lyric_compiler::CompilerCondition::kInvalidMutation:
                    return "Invalid mutation";
                case lyric_compiler::CompilerCondition::kInvalidAccess:
                    return "Invalid access";
                case lyric_compiler::CompilerCondition::kImplConflict:
                    return "Impl conflict";
                case lyric_compiler::CompilerCondition::kSymbolAlreadyDefined:
                    return "Symbol already defined";
                case lyric_compiler::CompilerCondition::kImportError:
                    return "Import error";
                case lyric_compiler::CompilerCondition::kSyntaxError:
                    return "Syntax error";
                case lyric_compiler::CompilerCondition::kCompilerInvariant:
                    return "Compiler invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_COMPILER_COMPILER_RESULT_H
