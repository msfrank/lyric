
#include <absl/strings/substitute.h>

#include <lyric_archiver/archiver_result.h>

lyric_archiver::ArchiverStatus::ArchiverStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<ArchiverCondition>(statusCode, detail)
{
}

lyric_archiver::ArchiverStatus
lyric_archiver::ArchiverStatus::ok()
{
    return ArchiverStatus();
}

bool
lyric_archiver::ArchiverStatus::convert(ArchiverStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricArchiverStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = ArchiverStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
