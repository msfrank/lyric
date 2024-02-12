#ifndef LYRIC_TEST_TEST_RUN_MATCHERS_H
#define LYRIC_TEST_TEST_RUN_MATCHERS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/parse_result.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_test/test_runner.h>
#include <lyric_test/test_run.h>

using ::testing::Matcher;

namespace lyric_test {

    namespace matchers {

        class TestComputationMatcher {

                enum class Type {
                    Invalid,
                    MatchSpanset,
                    MatchStatus,
                };
        public:
            TestComputationMatcher(lyric_build::TaskState::Status status);
            TestComputationMatcher(const Matcher<tempo_tracing::TempoSpanset> &spansetMatcher);

            bool MatchAndExplain(const lyric_test::TestComputation &testComputation, std::ostream *os) const;
            void DescribeTo(std::ostream *os) const;
            void DescribeNegationTo(std::ostream *os) const;

            using MatchesType = lyric_test::TestComputation;
            using is_gtest_matcher = void;

        private:
            Type m_type;
            Matcher<tempo_tracing::TempoSpanset> m_matcher;
            lyric_build::TaskState::Status m_status;
        };

        Matcher<lyric_test::TestComputation> FailedComputation(const Matcher<tempo_tracing::TempoSpanset> &matcher);

//        class CompileModuleMatcher {
//
//        public:
//            CompileModuleMatcher(const Matcher<lyric_runtime::LyricAssembly> &matcher);
//            bool MatchAndExplain(const lyric_test::CompileModule &runModule, std::ostream *os) const;
//            void DescribeTo(std::ostream *os) const;
//            void DescribeNegationTo(std::ostream *os) const;
//
//            using MatchesType = lyric_test::CompileModule;
//            using is_gtest_matcher = void;
//
//        private:
//            Matcher<lyric_runtime::Return> m_matcher;
//        };
//
//        Matcher<lyric_test::RunModule> CompileModule(const Matcher<lyric_runtime::LyricAssembly> &matcher);
        Matcher<lyric_test::CompileModule> CompileModule(const Matcher<tempo_tracing::TempoSpanset> &matcher);

        class RunModuleMatcher {

        public:
            RunModuleMatcher(const Matcher<lyric_runtime::Return> &matcher);
            bool MatchAndExplain(const lyric_test::RunModule &runModule, std::ostream *os) const;
            void DescribeTo(std::ostream *os) const;
            void DescribeNegationTo(std::ostream *os) const;

            using MatchesType = lyric_test::RunModule;
            using is_gtest_matcher = void;

        private:
            Matcher<lyric_runtime::Return> m_matcher;
        };

        Matcher<lyric_test::RunModule> RunModule(const Matcher<lyric_runtime::Return> &matcher);
        Matcher<lyric_test::RunModule> RunModule(const Matcher<tempo_tracing::TempoSpanset> &matcher);
    }
}

namespace lyric_test {

    void PrintTo(const TestComputation &testComputation, std::ostream *os);
    std::ostream& operator<<(std::ostream& os, const TestComputation &testComputation);
}

#endif // LYRIC_TEST_TEST_RUN_MATCHERS_H