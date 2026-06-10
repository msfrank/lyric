
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <absl/strings/str_split.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/status_ref.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_test/status_ref_matchers.h>
#include <lyric_test/test_result.h>
#include <tempo_utils/unicode.h>

lyric_test::matchers::StatusRefMatcher::StatusRefMatcher()
    : m_type(MatcherType::INVALID)
{
}

lyric_test::matchers::StatusRefMatcher::StatusRefMatcher(const lyric_common::SymbolUrl &symbolUrl)
    : m_type(MatcherType::STATUS_SYMBOL),
      m_symbol(symbolUrl)
{
}

lyric_test::matchers::StatusRefMatcher::StatusRefMatcher(tempo_utils::StatusCode statusCode)
    : m_type(MatcherType::STATUS_CODE),
      m_code(statusCode)
{
}

lyric_test::matchers::StatusRefMatcher::StatusRefMatcher(std::string_view message)
    : m_type(MatcherType::STATUS_MESSAGE),
      m_message(message)
{
}

lyric_test::matchers::StatusRefMatcher::StatusRefMatcher(const StatusRefMatcher &other)
    : m_type(other.m_type),
      m_symbol(other.m_symbol),
      m_code(other.m_code),
      m_message(other.m_message)
{
}

bool
lyric_test::matchers::StatusRefMatcher::MatchAndExplain(
    const lyric_runtime::Operand &cell,
    std::ostream* os) const
{
    switch (m_type) {
        case MatcherType::INVALID:
            return false;
        case MatcherType::STATUS_MESSAGE: {
            lyric_runtime::StatusRef *status;
            if (!cell.getStatus(status))
                return false;
            auto message = status->statusMessage();
            return m_message == message;
        }
        case MatcherType::STATUS_CODE: {
            lyric_runtime::StatusRef *status;
            if (!cell.getStatus(status))
                return false;
            auto statusCode = status->statusCode();
            return m_code == statusCode;
        }
        case MatcherType::STATUS_SYMBOL: {
            lyric_runtime::StatusRef *status;
            if (!cell.getStatus(status))
                return false;
            auto *vtable = status->getVirtualTable();
            if (vtable == nullptr)
                return false;
            auto sym = vtable->getSymbolUrl();
            if (!m_symbol.getModuleLocation().isValid())
                return sym.getSymbolPath() == m_symbol.getSymbolPath();
            return m_symbol == sym;
        }
    }
    TU_UNREACHABLE();
}

void
lyric_test::matchers::StatusRefMatcher::DescribeTo(std::ostream* os) const
{
    switch (m_type) {
        case MatcherType::STATUS_MESSAGE:
            *os << "status message matches string " << m_message;
            break;
        case MatcherType::STATUS_CODE:
            *os << "status code matches " << tempo_utils::status_code_to_string(m_code);
            break;
        case MatcherType::STATUS_SYMBOL:
            *os << "status is instance of " << m_symbol.toString();
            break;
        default:
            *os << "!!! invalid matcher !!!";
    }
}

void
lyric_test::matchers::StatusRefMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "status does not match";
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::StatusRef(const lyric_common::SymbolUrl &symbolUrl)
{
    return StatusRefMatcher(symbolUrl);
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::StatusRef(const lyric_common::SymbolPath &symbolPath)
{
    return StatusRefMatcher(lyric_common::SymbolUrl(symbolPath));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::MatchesStatusRefCode(tempo_utils::StatusCode statusCode)
{
    return StatusRefMatcher(statusCode);
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::MatchesStatusRefMessage(std::string_view message)
{
    return StatusRefMatcher(message);
}