#include <absl/strings/substitute.h>

#include <lyric_assembler/assembler_result.h>

lyric_assembler::AssemblerStatus::AssemblerStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<AssemblerCondition>(statusCode, detail)
{
}

lyric_assembler::AssemblerStatus
lyric_assembler::AssemblerStatus::ok()
{
    return AssemblerStatus();
}

bool
lyric_assembler::AssemblerStatus::convert(AssemblerStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricAssemblerStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = AssemblerStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}