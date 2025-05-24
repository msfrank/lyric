#include <absl/strings/substitute.h>

#include <lyric_importer/importer_result.h>

lyric_importer::ImporterStatus::ImporterStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<ImporterCondition>(statusCode, detail)
{
}

lyric_importer::ImporterStatus
lyric_importer::ImporterStatus::ok()
{
    return ImporterStatus();
}

bool
lyric_importer::ImporterStatus::convert(ImporterStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricImporterStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = ImporterStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}