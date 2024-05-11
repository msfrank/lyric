
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <absl/strings/str_split.h>

#include <lyric_build/build_types.h>
#include <lyric_test/computation_matchers.h>

lyric_test::matchers::ComputationMatcher::ComputationMatcher()
    : m_tester(nullptr)
{
}

lyric_test::matchers::ComputationMatcher::ComputationMatcher(
    const Matcher<tempo_tracing::TempoSpanset> &spansetMatcher,
    lyric_test::TestRunner *tester)
    : m_spansetMatcher(spansetMatcher),
      m_tester(tester)
{
    TU_ASSERT (m_tester != nullptr);
}

lyric_test::matchers::ComputationMatcher::ComputationMatcher(const ComputationMatcher &other)
    : m_spansetMatcher(other.m_spansetMatcher),
      m_tester(other.m_tester)
{
}

bool
lyric_test::matchers::ComputationMatcher::MatchAndExplain(
    const lyric_build::TargetComputation &computation,
    std::ostream* os) const
{
    auto state = computation.getState();
    if (state.getStatus() != lyric_build::TaskState::Status::FAILED)
        return false;
    if (m_tester == nullptr)
        return true;

    auto diagnostics = m_tester->getDiagnostics(computation);
    return m_spansetMatcher.Matches(diagnostics);
}

void
lyric_test::matchers::ComputationMatcher::DescribeTo(std::ostream* os) const
{
    if (m_tester == nullptr) {
        *os << "computation is failed";
    } else {
        *os << "computation is failed and ";
        m_spansetMatcher.DescribeTo(os);
    }
}

void
lyric_test::matchers::ComputationMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "return does not match";
}

Matcher<lyric_build::TargetComputation>
lyric_test::matchers::IsFailedComputation()
{
    return ComputationMatcher();
}

Matcher<lyric_build::TargetComputation>
lyric_test::matchers::IsFailedComputation(
    lyric_test::TestRunner *tester,
    const Matcher<tempo_tracing::TempoSpanset> &matcher)
{
    return ComputationMatcher(matcher, tester);
}

std::ostream&
lyric_build::operator<<(std::ostream& os, const TargetComputation &computation)
{
    return os << "TargetComputation for "
        << computation.getId().toString()
        << " with state " << computation.getState().toString();
}

void
lyric_build::PrintTo(const TargetComputation &computation, std::ostream *os)
{
    *os << "TargetComputation for "
        << computation.getId().toString()
        << " with state " << computation.getState().toString();
}
