
#include <lyric_analyzer/analyzer_result.h>

lyric_analyzer::AnalyzerStatus::AnalyzerStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<AnalyzerCondition>(statusCode, detail)
{
}

lyric_analyzer::AnalyzerStatus
lyric_analyzer::AnalyzerStatus::ok()
{
    return AnalyzerStatus();
}

bool
lyric_analyzer::AnalyzerStatus::convert(AnalyzerStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricAnalyzerStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = AnalyzerStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}