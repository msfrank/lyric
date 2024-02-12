#ifndef LYRIC_TEST_DIAGNOSTIC_MATCHERS_H
#define LYRIC_TEST_DIAGNOSTIC_MATCHERS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/build_types.h>
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

        class DiagnosticMatcher {

            enum class MatcherType {
                INVALID,
                DATA_CELL,
                REF_HANDLE,
            };

        public:
            DiagnosticMatcher();
            DiagnosticMatcher(const tempo_utils::Status &status);
            DiagnosticMatcher(const DiagnosticMatcher &other);

            bool MatchAndExplain(const tempo_tracing::TempoSpanset &spanset, std::ostream *os) const;
            void DescribeTo(std::ostream *os) const;
            void DescribeNegationTo(std::ostream *os) const;

            using MatchesType = tempo_tracing::TempoSpanset;
            using is_gtest_matcher = void;

        private:
            tempo_utils::Status m_status;
        };

        template<typename ConditionType,
            typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        Matcher<tempo_tracing::TempoSpanset> SpansetContainsError(ConditionType conditionType)
        {
            auto status = StatusType::forCondition(conditionType);
            return DiagnosticMatcher(status);
        }
    }
}

namespace tempo_tracing {
    void PrintTo(const TempoSpanset &spanset, std::ostream *os);
    std::ostream& operator<<(std::ostream& os, const TempoSpanset &spanset);
}

#endif // LYRIC_TEST_DIAGNOSTIC_MATCHERS_H