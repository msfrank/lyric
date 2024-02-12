
#include <lyric_test/test_result.h>

lyric_test::TestStatus::TestStatus(
    tempo_utils::StatusCode statusCode,
    std::shared_ptr<const tempo_utils::Detail> detail)
    : tempo_utils::TypedStatus<TestCondition>(statusCode, detail)
{
}

lyric_test::TestStatus
lyric_test::TestStatus::ok()
{
    return TestStatus();
}

bool
lyric_test::TestStatus::convert(TestStatus &dstStatus, const tempo_utils::Status &srcStatus)
{
    std::string_view srcNs = srcStatus.getErrorCategory();
    std::string_view dstNs = kLyricTestStatusNs.getNs();
    if (srcNs != dstNs)
        return false;
    dstStatus = TestStatus(srcStatus.getStatusCode(), srcStatus.getDetail());
    return true;
}

lyric_test::TestException::TestException(const TestStatus &status) noexcept
    : m_status(status)
{
}

lyric_test::TestStatus
lyric_test::TestException::getStatus() const
{
    return m_status;
}

const char *
lyric_test::TestException::what() const noexcept
{
    return m_status.getMessage().data();
}