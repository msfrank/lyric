#ifndef LYRIC_TEST_STATUS_REF_MATCHERS_H
#define LYRIC_TEST_STATUS_REF_MATCHERS_H

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

        class StatusRefMatcher {

            enum class MatcherType {
                INVALID,
                STATUS_SYMBOL,
                STATUS_CODE,
                STATUS_MESSAGE,
            };

        public:
            StatusRefMatcher();
            StatusRefMatcher(const lyric_common::SymbolUrl &symbolUrl);
            StatusRefMatcher(tempo_utils::StatusCode statusCode);
            StatusRefMatcher(std::string_view message);
            StatusRefMatcher(const StatusRefMatcher &other);

            bool MatchAndExplain(const lyric_runtime::DataCell &cell, std::ostream *os) const;
            void DescribeTo(std::ostream *os) const;
            void DescribeNegationTo(std::ostream *os) const;

            using MatchesType = lyric_runtime::DataCell;
            using is_gtest_matcher = void;

        private:
            MatcherType m_type;
            lyric_common::SymbolUrl m_symbol;
            tempo_utils::StatusCode m_code;
            std::string m_message;
        };

        Matcher<lyric_runtime::DataCell> StatusRef(const lyric_common::SymbolUrl &symbolUrl);
        Matcher<lyric_runtime::DataCell> StatusRef(const lyric_common::SymbolPath &symbolPath);
        Matcher<lyric_runtime::DataCell> MatchesStatusRefCode(tempo_utils::StatusCode statusCode);
        Matcher<lyric_runtime::DataCell> MatchesStatusRefMessage(std::string_view message);
    }
}

#endif // LYRIC_TEST_STATUS_REF_MATCHERS_H