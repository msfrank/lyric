
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <absl/strings/str_split.h>

#include <lyric_test/status_matchers.h>
#include <lyric_test/test_result.h>
#include <tempo_utils/unicode.h>

lyric_test::matchers::StatusMatcher::StatusMatcher()
    : m_type(MatcherType::INVALID)
{
}

lyric_test::matchers::StatusMatcher::StatusMatcher(tempo_utils::StatusCode statusCode)
    : m_type(MatcherType::STATUS_CODE),
      m_statusCode(statusCode)
{
}

lyric_test::matchers::StatusMatcher::StatusMatcher(std::string_view errorCategory)
    : m_type(MatcherType::ERROR_CATEGORY),
      m_errorCategory(errorCategory)
{
}

lyric_test::matchers::StatusMatcher::StatusMatcher(std::string_view errorCategory, int errorCode)
    : m_type(MatcherType::ERROR_CATEGORY_AND_CODE),
      m_errorCategory(errorCategory),
      m_errorCode(errorCode)
{
}

lyric_test::matchers::StatusMatcher::StatusMatcher(const StatusMatcher &other)
    : m_type(other.m_type),
      m_statusCode(other.m_statusCode),
      m_errorCategory(other.m_errorCategory),
      m_errorCode(other.m_errorCode)
{
}

bool
lyric_test::matchers::StatusMatcher::MatchAndExplain(
    const tempo_utils::Status &status,
    std::ostream* os) const
{
    switch (m_type) {
        case MatcherType::STATUS_CODE: {
            return status.getStatusCode() == m_statusCode;
        }
        case MatcherType::ERROR_CATEGORY: {
            return status.getErrorCategory() == m_errorCategory;
        }
        case MatcherType::ERROR_CATEGORY_AND_CODE: {
            return status.getErrorCategory() == m_errorCategory && status.getErrorCode() == m_errorCode;
        }
        default:
            return false;
    }
}

void
lyric_test::matchers::StatusMatcher::DescribeTo(std::ostream* os) const
{
    switch (m_type) {
        case MatcherType::STATUS_CODE:
            *os << "status matches status code " << std::string_view(tempo_utils::status_code_to_string(m_statusCode));
            break;
        case MatcherType::ERROR_CATEGORY:
            *os << "status matches error category " << m_errorCategory;
            break;
        case MatcherType::ERROR_CATEGORY_AND_CODE:
            *os << "status contains error category " << m_errorCategory << " and code " << m_errorCode;
            break;
        default:
            *os << "!!! invalid matcher !!!";
    }
}

void
lyric_test::matchers::StatusMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "status does not match";
}

Matcher<tempo_utils::Status>
lyric_test::matchers::MatchesStatusCode(tempo_utils::StatusCode statusCode)
{
    return StatusMatcher(statusCode);
}

Matcher<tempo_utils::Status>
lyric_test::matchers::MatchesErrorCategory(std::string_view errorCategory)
{
    return StatusMatcher(errorCategory);
}

Matcher<tempo_utils::Status>
lyric_test::matchers::MatchesErrorCategory(const tempo_utils::SchemaNs &errorCategory)
{
    return StatusMatcher(errorCategory.getNs());
}

std::ostream&
tempo_utils::operator<<(std::ostream& os, const Status &status)
{
    return os << status.toString();
}

void
tempo_utils::PrintTo(const Status &status, std::ostream *os)
{
    *os << status.toString();
}