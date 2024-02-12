#ifndef LYRIC_TEST_RETURN_MATCHERS_H
#define LYRIC_TEST_RETURN_MATCHERS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/parse_result.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_test/data_cell_matchers.h>
#include <lyric_test/ref_handle_matchers.h>
#include <lyric_test/test_result.h>

using ::testing::Matcher;

namespace lyric_test {

    namespace matchers {

        class ReturnMatcher {

            enum class MatcherType {
                INVALID,
                DATA_CELL,
                REF_HANDLE,
            };

        public:
            ReturnMatcher();
            ReturnMatcher(const Matcher<lyric_runtime::DataCell> &matcher);
            ReturnMatcher(const Matcher<lyric_runtime::RefHandle> &matcher);
            ReturnMatcher(const ReturnMatcher &other);

            bool MatchAndExplain(const lyric_runtime::Return &ret, std::ostream *os) const;
            void DescribeTo(std::ostream *os) const;
            void DescribeNegationTo(std::ostream *os) const;

            using MatchesType = lyric_runtime::Return;
            using is_gtest_matcher = void;

        private:
            MatcherType m_type;
            Matcher<lyric_runtime::DataCell> m_dataCellMatcher;
            Matcher<lyric_runtime::RefHandle> m_refHandleMatcher;
        };

        Matcher<lyric_runtime::Return> Return(const Matcher<lyric_runtime::DataCell> &matcher);
        Matcher<lyric_runtime::Return> Return(const Matcher<lyric_runtime::RefHandle> &matcher);
    }
}

namespace lyric_runtime {
    void PrintTo(const Return &ret, std::ostream *os);
    std::ostream& operator<<(std::ostream& os, const Return &ret);
}

#endif // LYRIC_TEST_RETURN_MATCHERS_H