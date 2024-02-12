#ifndef LYRIC_TEST_REF_HANDLE_MATCHERS_H
#define LYRIC_TEST_REF_HANDLE_MATCHERS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/parse_result.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_test/test_result.h>

using ::testing::Matcher;

namespace lyric_test {

    namespace matchers {

        class RefHandleMatcher {

        public:
            RefHandleMatcher();
            RefHandleMatcher(const lyric_common::SymbolUrl &symbolUrl);
            RefHandleMatcher(const RefHandleMatcher &other);

            bool MatchAndExplain(const lyric_runtime::RefHandle &refHandle, std::ostream *os) const;
            void DescribeTo(std::ostream *os) const;
            void DescribeNegationTo(std::ostream *os) const;

            using MatchesType = lyric_runtime::RefHandle;
            using is_gtest_matcher = void;

        private:
            lyric_common::SymbolUrl m_symbolUrl;
        };

        Matcher<lyric_runtime::RefHandle> IsRefType(const lyric_common::SymbolUrl &symbolUrl);
        Matcher<lyric_runtime::RefHandle> IsRefType(const lyric_common::SymbolPath &symbolPath);
    }
}

namespace lyric_runtime {
    void PrintTo(const lyric_runtime::RefHandle &refHandle, std::ostream *os);
    std::ostream& operator<<(std::ostream& os, const RefHandle &handle);
}

#endif // LYRIC_TEST_REF_HANDLE_MATCHERS_H