
#include <absl/strings/substitute.h>

#include <lyric_serde/serde_result.h>

lyric_serde::SerdeStatus::SerdeStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<SerdeCondition>(statusCode, detail)
{
}

lyric_serde::SerdeStatus
lyric_serde::SerdeStatus::ok()
{
    return SerdeStatus();
}

bool
lyric_serde::SerdeStatus::convert(SerdeStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricSerdeStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = SerdeStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
