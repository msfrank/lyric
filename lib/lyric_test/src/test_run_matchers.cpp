
#include <lyric_test/test_run_matchers.h>

lyric_test::matchers::TestComputationMatcher::TestComputationMatcher(lyric_build::TaskState::Status status)
    : m_type(Type::MatchStatus),
      m_status(status)
{
}

lyric_test::matchers::TestComputationMatcher::TestComputationMatcher(
    const Matcher<tempo_tracing::TempoSpanset> &spansetMatcher)
    : m_type(Type::MatchSpanset),
      m_matcher(spansetMatcher)
{
}

bool
lyric_test::matchers::TestComputationMatcher::MatchAndExplain(
    const lyric_test::TestComputation &testComputation,
    std::ostream *os) const
{
    if (!testComputation.hasComputation())
        return false;
    auto computation = testComputation.getComputation();
    switch (m_type) {
        case Type::MatchStatus:
            return computation.getState().getStatus() == m_status;
        case Type::MatchSpanset:
            return m_matcher.Matches(testComputation.getDiagnostics()->getSpanset());
        default:
            return false;
    }
}

inline std::string status_to_string(lyric_build::TaskState::Status status)
{
    switch (status) {
        case lyric_build::TaskState::Status::COMPLETED:
            return "COMPLETED";
        case lyric_build::TaskState::Status::FAILED:
            return "FAILED";
        case lyric_build::TaskState::Status::BLOCKED:
            return "BLOCKED";
        case lyric_build::TaskState::Status::QUEUED:
            return "QUEUED";
        case lyric_build::TaskState::Status::RUNNING:
            return "RUNNING";
        case lyric_build::TaskState::Status::INVALID:
            return "INVALID";
    }
}

void
lyric_test::matchers::TestComputationMatcher::DescribeTo(std::ostream *os) const
{
    switch (m_type) {
        case Type::MatchStatus:
            *os << "computation status is " << status_to_string(m_status);
            break;
        case Type::MatchSpanset:
            m_matcher.DescribeTo(os);
            break;
        default:
            *os << "!!! invalid matcher !!!";
            break;
    }
}

void
lyric_test::matchers::TestComputationMatcher::DescribeNegationTo(std::ostream *os) const
{
    switch (m_type) {
        case Type::MatchStatus:
            *os << "computation status is not " << status_to_string(m_status);
            break;
        case Type::MatchSpanset:
            m_matcher.DescribeNegationTo(os);
            break;
        default:
            *os << "!!! invalid matcher !!!";
            break;
    }
}

void
lyric_test::PrintTo(const TestComputation &testComputation, std::ostream *os)
{
    if (testComputation.hasComputation()) {
        auto computation = testComputation.getComputation();
        *os << "TestComputation for "
            << computation.getId().toString()
            << " with state " << computation.getState().toString();
    } else {
        *os << "empty TestComputation";
    }
}

std::ostream&
lyric_test::operator<<(std::ostream& os, const TestComputation &testComputation)
{
    if (testComputation.hasComputation()) {
        auto computation = testComputation.getComputation();
        return os << "TestComputation for "
                  << computation.getId().toString()
                  << " with state " << computation.getState().toString();
    } else {
        return os << "empty TestComputation";
    }
}

Matcher<lyric_test::SymbolizeModule>
lyric_test::matchers::SymbolizeModule(lyric_build::TaskState::Status status)
{
    return TestComputationMatcher(status);
}

Matcher<lyric_test::SymbolizeModule>
lyric_test::matchers::SymbolizeModule(const Matcher<tempo_tracing::TempoSpanset> &matcher)
{
    return TestComputationMatcher(matcher);
}

Matcher<lyric_test::AnalyzeModule>
lyric_test::matchers::AnalyzeModule(lyric_build::TaskState::Status status)
{
    return TestComputationMatcher(status);
}

Matcher<lyric_test::AnalyzeModule>
lyric_test::matchers::AnalyzeModule(const Matcher<tempo_tracing::TempoSpanset> &matcher)
{
    return TestComputationMatcher(matcher);
}

Matcher<lyric_test::CompileModule>
lyric_test::matchers::CompileModule(lyric_build::TaskState::Status status)
{
    return TestComputationMatcher(status);
}

Matcher<lyric_test::CompileModule>
lyric_test::matchers::CompileModule(const Matcher<tempo_tracing::TempoSpanset> &matcher)
{
    return TestComputationMatcher(matcher);
}

lyric_test::matchers::RunModuleMatcher::RunModuleMatcher(const Matcher<lyric_runtime::DataCell> &matcher)
    : m_matcher(matcher)
{
}

bool
lyric_test::matchers::RunModuleMatcher::MatchAndExplain(
    const lyric_test::RunModule &runModule,
    std::ostream *os) const
{
    if (!runModule.hasInterpreterState())
        return false;
    auto exit = runModule.getInterpreterExit();
    return m_matcher.Matches(exit.mainReturn);
}

void
lyric_test::matchers::RunModuleMatcher::DescribeTo(std::ostream *os) const
{
    m_matcher.DescribeTo(os);
}

void
lyric_test::matchers::RunModuleMatcher::DescribeNegationTo(std::ostream *os) const
{
    m_matcher.DescribeNegationTo(os);
}

Matcher<lyric_test::RunModule>
lyric_test::matchers::RunModule(const Matcher<lyric_runtime::DataCell> &matcher)
{
    return RunModuleMatcher(matcher);
}

Matcher<lyric_test::RunModule>
lyric_test::matchers::RunModule(const Matcher<tempo_tracing::TempoSpanset> &matcher)
{
    //return RunModuleMatcher(matcher);
    return TestComputationMatcher(matcher);
}

