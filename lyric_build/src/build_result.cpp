
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

lyric_build::BuildException::BuildException(const BuildStatus &status) noexcept
    : m_status(status)
{
}

lyric_build::BuildStatus
lyric_build::BuildException::getStatus() const
{
    return m_status;
}

const char *
lyric_build::BuildException::what() const noexcept
{
    return m_status.getMessage().data();
}