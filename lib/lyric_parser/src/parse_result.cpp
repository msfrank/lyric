
#include <lyric_parser/parse_result.h>

lyric_parser::ParseStatus::ParseStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<ParseCondition>(statusCode, detail)
{
}

bool
lyric_parser::ParseStatus::convert(ParseStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricParserStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = ParseStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}