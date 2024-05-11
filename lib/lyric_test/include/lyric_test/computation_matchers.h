#ifndef LYRIC_TEST_COMPUTATION_MATCHERS_H
#define LYRIC_TEST_COMPUTATION_MATCHERS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/build_types.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/parse_result.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_result.h>

#include "test_runner.h"
#include "test_result.h"

using ::testing::Matcher;

namespace lyric_test {

    namespace matchers {

        class ComputationMatcher {

        public:
            ComputationMatcher();
            ComputationMatcher(
                const Matcher<tempo_tracing::TempoSpanset> &spansetMatcher,
                lyric_test::TestRunner *tester);
            ComputationMatcher(const ComputationMatcher &other);

            bool MatchAndExplain(const lyric_build::TargetComputation &computation, std::ostream *os) const;
            void DescribeTo(std::ostream *os) const;
            void DescribeNegationTo(std::ostream *os) const;

            using MatchesType = lyric_build::TargetComputation;
            using is_gtest_matcher = void;

        private:
            Matcher<tempo_tracing::TempoSpanset> m_spansetMatcher;
            lyric_test::TestRunner *m_tester;
        };

        Matcher<lyric_build::TargetComputation> IsFailedComputation();
        Matcher<lyric_build::TargetComputation> IsFailedComputation(
            lyric_test::TestRunner *tester,
            const Matcher<tempo_tracing::TempoSpanset> &matcher);
    }
}

namespace lyric_build {
    void PrintTo(const TargetComputation &computation, std::ostream *os);
    std::ostream& operator<<(std::ostream& os, const TargetComputation &computation);
}

#endif // LYRIC_TEST_COMPUTATION_MATCHERS_H