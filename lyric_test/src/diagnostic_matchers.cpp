
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <absl/strings/str_split.h>

#include <lyric_test/diagnostic_matchers.h>
#include <lyric_test/test_result.h>
#include <tempo_tracing/error_walker.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/unicode.h>

lyric_test::matchers::DiagnosticMatcher::DiagnosticMatcher()
{
}

lyric_test::matchers::DiagnosticMatcher::DiagnosticMatcher(const tempo_utils::Status &status)
    : m_status(status)
{
}

lyric_test::matchers::DiagnosticMatcher::DiagnosticMatcher(const DiagnosticMatcher &other)
    : m_status(other.m_status)
{
}

bool
lyric_test::matchers::DiagnosticMatcher::MatchAndExplain(
    const tempo_tracing::TempoSpanset &spanset,
    std::ostream* os) const
{
    auto errorWalker = spanset.getErrors();

    for (int i = 0; i < errorWalker.numErrors(); i++) {
        auto spanWalker = errorWalker.getError(i);
        for (int j = 0; j < spanWalker.numLogs(); j++) {
            auto logWalker = spanWalker.getLog(j);
            if (logWalker.getSeverity() != tempo_tracing::LogSeverity::kError)
                continue;
            std::string errorCategory;
            logWalker.parseField(tempo_tracing::kTempoTracingErrorCategoryName, errorCategory);
            tu_int64 errorCode;
            logWalker.parseField(tempo_tracing::kTempoTracingErrorCode, errorCode);

            if (m_status.getErrorCategory() == errorCategory && m_status.getErrorCode() == errorCode)
                return true;
        }
    }

    return false;
}

void
lyric_test::matchers::DiagnosticMatcher::DescribeTo(std::ostream* os) const
{
    *os << "spanset contains status " << m_status.toString();
}

void
lyric_test::matchers::DiagnosticMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "spanset does not contain status " << m_status.toString();
}

std::ostream&
tempo_tracing::operator<<(std::ostream& os, const TempoSpanset &spanset)
{
    return os << "TempoSpanset";
}

void
tempo_tracing::PrintTo(const TempoSpanset &spanset, std::ostream *os)
{
    *os << "TempoSpanset";
}
