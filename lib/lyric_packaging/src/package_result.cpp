
#include <absl/strings/substitute.h>

#include <lyric_packaging/package_result.h>

lyric_packaging::PackageStatus::PackageStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<PackageCondition>(statusCode, detail)
{
}

lyric_packaging::PackageStatus
lyric_packaging::PackageStatus::ok()
{
    return PackageStatus();
}

bool
lyric_packaging::PackageStatus::convert(PackageStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricPackagingStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = PackageStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
