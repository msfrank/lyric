#include <absl/strings/substitute.h>

#include <lyric_compiler/compiler_result.h>

lyric_compiler::CompilerStatus::CompilerStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<CompilerCondition>(statusCode, detail)
{
}

lyric_compiler::CompilerStatus
lyric_compiler::CompilerStatus::ok()
{
    return CompilerStatus();
}

bool
lyric_compiler::CompilerStatus::convert(CompilerStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricCompilerStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = CompilerStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}

lyric_compiler::CompilerException::CompilerException(const CompilerStatus &status) noexcept
    : m_status(status)
{
}

lyric_compiler::CompilerStatus
lyric_compiler::CompilerException::getStatus() const
{
    return m_status;
}

const char *
lyric_compiler::CompilerException::what() const noexcept
{
    return m_status.getMessage().data();
}
