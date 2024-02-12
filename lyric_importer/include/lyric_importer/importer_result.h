#ifndef LYRIC_IMPORTER_COMPILER_RESULT_H
#define LYRIC_IMPORTER_COMPILER_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/log_stream.h>
#include <tempo_utils/status.h>

namespace lyric_importer {

    constexpr tempo_utils::SchemaNs kLyricImporterStatusNs("dev.zuri.ns:lyric-importer-status-1");

    enum class ImporterCondition {
        kTypeError,
        kIncompatibleType,
        kMissingType,
        kMissingTemplate,
        kMissingSymbol,
        kModuleNotFound,
        kInvalidSymbol,
        kSymbolAlreadyDefined,
        kImportError,
        kImporterInvariant,
    };

    class ImporterStatus : public tempo_utils::TypedStatus<ImporterCondition> {
    public:
        using TypedStatus::TypedStatus;

        static ImporterStatus ok();

        static bool convert(ImporterStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        ImporterStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

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
        static ImporterStatus forCondition(
            ImporterCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ImporterStatus(condition, message);
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
        static ImporterStatus forCondition(
            ImporterCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return ImporterStatus(condition, message, traceId, spanId);
        }
    };

    class ImporterException : public std::exception {
    public:
        ImporterException(const ImporterStatus &status) noexcept;
        ImporterStatus getStatus() const;
        const char* what() const noexcept override;

    private:
        ImporterStatus m_status;
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_importer::ImporterCondition> {
        using ConditionType = lyric_importer::ImporterCondition;
        static bool convert(lyric_importer::ImporterStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_importer::ImporterStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_importer::ImporterCondition> {
        using StatusType = lyric_importer::ImporterStatus;
        static constexpr const char *condition_namespace() { return lyric_importer::kLyricImporterStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_importer::ImporterCondition condition)
        {
            switch (condition) {
                case lyric_importer::ImporterCondition::kTypeError:
                case lyric_importer::ImporterCondition::kIncompatibleType:
                case lyric_importer::ImporterCondition::kMissingType:
                case lyric_importer::ImporterCondition::kMissingTemplate:
                case lyric_importer::ImporterCondition::kMissingSymbol:
                case lyric_importer::ImporterCondition::kModuleNotFound:
                case lyric_importer::ImporterCondition::kInvalidSymbol:
                case lyric_importer::ImporterCondition::kSymbolAlreadyDefined:
                case lyric_importer::ImporterCondition::kImportError:
                case lyric_importer::ImporterCondition::kImporterInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_importer::ImporterCondition condition)
        {
            switch (condition) {
                case lyric_importer::ImporterCondition::kTypeError:
                    return "Type error";
                case lyric_importer::ImporterCondition::kIncompatibleType:
                    return "Incompatible type";
                case lyric_importer::ImporterCondition::kMissingType:
                    return "Missing type";
                case lyric_importer::ImporterCondition::kMissingTemplate:
                    return "Missing template";
                case lyric_importer::ImporterCondition::kMissingSymbol:
                    return "Missing symbol";
                case lyric_importer::ImporterCondition::kModuleNotFound:
                    return "Module not found";
                case lyric_importer::ImporterCondition::kInvalidSymbol:
                    return "Invalid symbol";
                case lyric_importer::ImporterCondition::kSymbolAlreadyDefined:
                    return "Symbol already defined";
                case lyric_importer::ImporterCondition::kImportError:
                    return "Import error";
                case lyric_importer::ImporterCondition::kImporterInvariant:
                    return "Importer invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_IMPORTER_COMPILER_RESULT_H