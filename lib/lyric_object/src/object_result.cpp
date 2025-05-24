#include <absl/strings/substitute.h>

#include <lyric_object/object_result.h>

lyric_object::ObjectStatus::ObjectStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<ObjectCondition>(statusCode, detail)
{
}

lyric_object::ObjectStatus
lyric_object::ObjectStatus::ok()
{
    return ObjectStatus();
}

bool
lyric_object::ObjectStatus::convert(ObjectStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricObjectStatusNs;
    if (srcNs != dstNs)
        return false;
    dstStatus = ObjectStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}
