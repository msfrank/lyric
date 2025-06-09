
#include <lyric_analyzer/analyzer_result.h>

lyric_analyzer::AnalyzerStatus::AnalyzerStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<AnalyzerCondition>(statusCode, detail)
{
}

bool
lyric_analyzer::AnalyzerStatus::convert(AnalyzerStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricAnalyzerStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = AnalyzerStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}