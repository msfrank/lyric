
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <absl/strings/str_split.h>

#include <lyric_test/ref_handle_matchers.h>
#include <lyric_test/test_result.h>

lyric_test::matchers::RefHandleMatcher::RefHandleMatcher()
    : m_symbolUrl()
{
}

lyric_test::matchers::RefHandleMatcher::RefHandleMatcher(const lyric_common::SymbolUrl &symbolUrl)
    : m_symbolUrl(symbolUrl)
{
}

lyric_test::matchers::RefHandleMatcher::RefHandleMatcher(const RefHandleMatcher &other)
    : m_symbolUrl(other.m_symbolUrl)
{
}

bool
lyric_test::matchers::RefHandleMatcher::MatchAndExplain(const lyric_runtime::RefHandle &refHandle, std::ostream* os) const
{
    auto *ref = refHandle.getRef();
    if (ref == nullptr)
        return false;
    auto refUrl = ref->getSymbolUrl();
    if (m_symbolUrl.getModuleLocation().isValid())
        return refUrl == m_symbolUrl;
    return refUrl.getSymbolPath() == m_symbolUrl.getSymbolPath();
}

void
lyric_test::matchers::RefHandleMatcher::DescribeTo(std::ostream* os) const
{
    *os << "result is an instance of " << m_symbolUrl.toString();
}

void
lyric_test::matchers::RefHandleMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "result is not an instance of " << m_symbolUrl.toString();
}

testing::Matcher<lyric_runtime::RefHandle>
lyric_test::matchers::IsRefType(const lyric_common::SymbolUrl &symbolUrl)
{
    return RefHandleMatcher(symbolUrl);
}

testing::Matcher<lyric_runtime::RefHandle>
lyric_test::matchers::IsRefType(const lyric_common::SymbolPath &symbolPath)
{
    return RefHandleMatcher(lyric_common::SymbolUrl(symbolPath));
}

std::ostream&
lyric_runtime::operator<<(std::ostream& os, const RefHandle &handle) {
    return os << handle.getRef()->toString();
}

void
lyric_runtime::PrintTo(const RefHandle &refHandle, std::ostream *os)
{
    auto *ref = refHandle.getRef();
    if (ref == nullptr) {
        *os << "nullptr";
    } else {
        *os << "RefHandle " << ref->toString();
    }
}
