#include <absl/strings/substitute.h>

#include <lyric_runtime/interpreter_result.h>

lyric_runtime::InterpreterStatus::InterpreterStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<InterpreterCondition>(statusCode, detail)
{
}

lyric_runtime::InterpreterStatus
lyric_runtime::InterpreterStatus::ok()
{
    return InterpreterStatus();
}

bool
lyric_runtime::InterpreterStatus::convert(InterpreterStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricRuntimeInterpreterStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = InterpreterStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
