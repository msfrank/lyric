
#include <absl/strings/substitute.h>

#include <lyric_bootstrap/bootstrap_result.h>

lyric_bootstrap::BootstrapStatus::BootstrapStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<BootstrapCondition>(statusCode, detail)
{
}

lyric_bootstrap::BootstrapStatus
lyric_bootstrap::BootstrapStatus::ok()
{
    return BootstrapStatus();
}

bool
lyric_bootstrap::BootstrapStatus::convert(BootstrapStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricBootstrapStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = BootstrapStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
