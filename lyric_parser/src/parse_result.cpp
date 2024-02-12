
#include <lyric_parser/parse_result.h>

lyric_parser::ParseStatus::ParseStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<ParseCondition>(statusCode, detail)
{
}

lyric_parser::ParseStatus
lyric_parser::ParseStatus::ok()
{
    return ParseStatus();
}

bool
lyric_parser::ParseStatus::convert(ParseStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricParserStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = ParseStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}

lyric_parser::ParseException::ParseException(const ParseStatus &status) noexcept
    : m_status(status)
{
}

lyric_parser::ParseStatus
lyric_parser::ParseException::getStatus() const
{
    return m_status;
}

const char *
lyric_parser::ParseException::what() const noexcept
{
    return m_status.getMessage().data();
}