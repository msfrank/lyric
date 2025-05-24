
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
    std::string_view dstNs = kLyricTypingStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = TypingStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}