
#include <lyric_build/build_result.h>

lyric_build::BuildStatus::BuildStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<BuildCondition>(statusCode, detail)
{
}

lyric_build::BuildStatus
lyric_build::BuildStatus::ok()
{
    return BuildStatus();
}

bool
lyric_build::BuildStatus::convert(BuildStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricBuildStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = BuildStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}