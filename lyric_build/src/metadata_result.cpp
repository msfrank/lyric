//
//#include <lyric_build/metadata_result.h>
//
//lyric_build::MetadataStatus::MetadataStatus(
//    tempo_utils::StatusCode statusCode,
//    std::shared_ptr<const tempo_utils::Detail> detail)
//    : tempo_utils::TypedStatus<MetadataCondition>(statusCode, detail)
//{
//}
//
//lyric_build::MetadataStatus
//lyric_build::MetadataStatus::ok()
//{
//    return MetadataStatus();
//}
//
//bool
//lyric_build::MetadataStatus::convert(MetadataStatus &dstStatus, const tempo_utils::Status &srcStatus)
//{
//    std::string_view srcNs = srcStatus.getErrorCategory();
//    std::string_view dstNs = kLyricBuildMetadataStatusNs.getNs();
//    if (srcNs != dstNs)
//        return false;
//    dstStatus = MetadataStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
//    return true;
//}
//
//lyric_build::MetadataException::MetadataException(const MetadataStatus &status) noexcept
//    : m_status(status)
//{
//}
//
//lyric_build::MetadataStatus
//lyric_build::MetadataException::getStatus() const
//{
//    return m_status;
//}
//
//const char *
//lyric_build::MetadataException::what() const noexcept
//{
//    return m_status.getMessage().data();
//}
