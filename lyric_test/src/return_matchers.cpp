
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <absl/strings/str_split.h>

#include <lyric_test/return_matchers.h>

lyric_test::matchers::ReturnMatcher::ReturnMatcher()
    : m_type(MatcherType::INVALID)
{
}

lyric_test::matchers::ReturnMatcher::ReturnMatcher(const Matcher<lyric_runtime::DataCell> &matcher)
    : m_type(MatcherType::DATA_CELL),
      m_dataCellMatcher(matcher)
{
}

lyric_test::matchers::ReturnMatcher::ReturnMatcher(const Matcher<lyric_runtime::RefHandle> &matcher)
    : m_type(MatcherType::REF_HANDLE),
      m_refHandleMatcher(matcher)
{
}

lyric_test::matchers::ReturnMatcher::ReturnMatcher(const ReturnMatcher &other)
    : m_type(other.m_type),
      m_dataCellMatcher(other.m_dataCellMatcher),
      m_refHandleMatcher(other.m_refHandleMatcher)
{
}

bool
lyric_test::matchers::ReturnMatcher::MatchAndExplain(
    const lyric_runtime::Return &ret,
    std::ostream* os) const
{
    switch (m_type) {
        case MatcherType::DATA_CELL:
            if (ret.type != lyric_runtime::ReturnType::VALUE)
                return false;
            return m_dataCellMatcher.Matches(ret.value);
        case MatcherType::REF_HANDLE:
            if (ret.type != lyric_runtime::ReturnType::REF)
                return false;
            return m_refHandleMatcher.Matches(ret.ref);
        case MatcherType::INVALID:
            return false;
    }
    TU_UNREACHABLE();
}

void
lyric_test::matchers::ReturnMatcher::DescribeTo(std::ostream* os) const
{
    switch (m_type) {
        case MatcherType::DATA_CELL:
            m_dataCellMatcher.DescribeTo(os);
            break;
        case MatcherType::REF_HANDLE:
            m_refHandleMatcher.DescribeTo(os);
            break;
        default:
            *os << "!!! invalid matcher !!!";
    }
}

void
lyric_test::matchers::ReturnMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "return does not match";
}

Matcher<lyric_runtime::Return>
lyric_test::matchers::Return(const Matcher<lyric_runtime::DataCell> &matcher)
{
    return ReturnMatcher(matcher);
}

Matcher<lyric_runtime::Return>
lyric_test::matchers::Return(const Matcher<lyric_runtime::RefHandle> &matcher)
{
    return ReturnMatcher(matcher);
}

std::ostream&
lyric_runtime::operator<<(std::ostream& os, const Return &ret)
{
    switch (ret.type) {
        case lyric_runtime::ReturnType::VALUE:
            return os << ret.value;
        case lyric_runtime::ReturnType::REF:
            return os << ret.ref;
        case lyric_runtime::ReturnType::EXCEPTION:
        case lyric_runtime::ReturnType::INVALID:
            return os << "!!! invalid matcher !!!";
    }
    TU_UNREACHABLE();
}

void
lyric_runtime::PrintTo(const Return &ret, std::ostream *os)
{
    switch (ret.type) {
        case lyric_runtime::ReturnType::VALUE:
            PrintTo(ret.value, os);
            break;;
        case lyric_runtime::ReturnType::REF:
            PrintTo(ret.ref, os);
            break;;
        case lyric_runtime::ReturnType::EXCEPTION:
        default:
            *os << "???";
            break;
    }
}
