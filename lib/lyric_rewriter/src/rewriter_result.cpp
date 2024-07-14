#include <absl/strings/substitute.h>

#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::RewriterStatus::RewriterStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<RewriterCondition>(statusCode, detail)
{
}

lyric_rewriter::RewriterStatus
lyric_rewriter::RewriterStatus::ok()
{
    return RewriterStatus();
}

bool
lyric_rewriter::RewriterStatus::convert(RewriterStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricRewriterStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = RewriterStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}