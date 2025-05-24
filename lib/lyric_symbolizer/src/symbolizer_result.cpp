#include <absl/strings/substitute.h>

#include <lyric_symbolizer/symbolizer_result.h>

lyric_symbolizer::SymbolizerStatus::SymbolizerStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<SymbolizerCondition>(statusCode, detail)
{
}

lyric_symbolizer::SymbolizerStatus
lyric_symbolizer::SymbolizerStatus::ok()
{
    return SymbolizerStatus();
}

bool
lyric_symbolizer::SymbolizerStatus::convert(SymbolizerStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricSymbolizerStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = SymbolizerStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}