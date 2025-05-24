
#include <absl/strings/substitute.h>

#include <lyric_optimizer/optimizer_result.h>

lyric_optimizer::OptimizerStatus::OptimizerStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<OptimizerCondition>(statusCode, detail)
{
}

lyric_optimizer::OptimizerStatus
lyric_optimizer::OptimizerStatus::ok()
{
    return OptimizerStatus();
}

bool
lyric_optimizer::OptimizerStatus::convert(OptimizerStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricOptimizerStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = OptimizerStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
