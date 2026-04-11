
#include <lyric_test/test_run_matchers.h>

lyric_test::matchers::TestComputationMatcher::TestComputationMatcher(lyric_build::TaskState state)
    : m_type(Type::MatchState),
      m_state(state)
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
        case Type::MatchState:
            return computation.getState().getState() == m_state;
        case Type::MatchSpanset:
            return m_matcher.Matches(testComputation.getDiagnostics()->getSpanset());
        default:
            return false;
    }
}

inline std::string task_state_to_string(lyric_build::TaskState state)
{
    switch (state) {
        case lyric_build::TaskState::COMPLETED:
            return "COMPLETED";
        case lyric_build::TaskState::FAILED:
            return "FAILED";
        case lyric_build::TaskState::BLOCKED:
            return "BLOCKED";
        case lyric_build::TaskState::QUEUED:
            return "QUEUED";
        case lyric_build::TaskState::RUNNING:
            return "RUNNING";
        case lyric_build::TaskState::INVALID:
            return "INVALID";
    }
}

void
lyric_test::matchers::TestComputationMatcher::DescribeTo(std::ostream *os) const
{
    switch (m_type) {
        case Type::MatchState:
            *os << "computation state is " << task_state_to_string(m_state);
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
        case Type::MatchState:
            *os << "computation state is not " << task_state_to_string(m_state);
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
lyric_test::matchers::SymbolizeModule(lyric_build::TaskState status)
{
    return TestComputationMatcher(status);
}

Matcher<lyric_test::SymbolizeModule>
lyric_test::matchers::SymbolizeModule(const Matcher<tempo_tracing::TempoSpanset> &matcher)
{
    return TestComputationMatcher(matcher);
}

Matcher<lyric_test::AnalyzeModule>
lyric_test::matchers::AnalyzeModule(lyric_build::TaskState status)
{
    return TestComputationMatcher(status);
}

Matcher<lyric_test::AnalyzeModule>
lyric_test::matchers::AnalyzeModule(const Matcher<tempo_tracing::TempoSpanset> &matcher)
{
    return TestComputationMatcher(matcher);
}

Matcher<lyric_test::CompileModule>
lyric_test::matchers::CompileModule(lyric_build::TaskState status)
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

