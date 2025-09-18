
#include <absl/strings/substitute.h>

#include <lyric_common/common_status.h>

lyric_common::CommonStatus::CommonStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<CommonCondition>(statusCode, detail)
{
}

bool
lyric_common::CommonStatus::convert(CommonStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricCommonStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = CommonStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
