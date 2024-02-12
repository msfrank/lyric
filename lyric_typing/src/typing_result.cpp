
#include <lyric_typing/typing_result.h>

lyric_typing::TypingStatus::TypingStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<TypingCondition>(statusCode, detail)
{
}

lyric_typing::TypingStatus
lyric_typing::TypingStatus::ok()
{
    return TypingStatus();
}

bool
lyric_typing::TypingStatus::convert(TypingStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricTypingStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = TypingStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}

lyric_typing::TypingException::TypingException(const TypingStatus &status) noexcept
    : m_status(status)
{
}

lyric_typing::TypingStatus
lyric_typing::TypingException::getStatus() const
{
    return m_status;
}

const char *
lyric_typing::TypingException::what() const noexcept
{
    return m_status.getMessage().data();
}
