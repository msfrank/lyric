#ifndef LYRIC_TEST_RESULT_MATCHERS_H
#define LYRIC_TEST_RESULT_MATCHERS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/parse_result.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_test/status_matchers.h>

using ::testing::Matcher;

namespace lyric_test {

    namespace matchers {

        template<typename MatchesType>
        class ResultMatcher {

        public:
            ResultMatcher(const Matcher<MatchesType> &matcher)
                : m_matcher(matcher) {};

            bool MatchAndExplain(const tempo_utils::Result<MatchesType> &result, std::ostream *os) const {
                if (result.isStatus())
                    return false;
                return m_matcher.Matches(result.getResult());
            }
            void DescribeTo(std::ostream *os) const {
                m_matcher.DescribeTo(os);
            }
            void DescribeNegationTo(std::ostream *os) const {
                m_matcher.DescribeNegationTo(os);
            }

            using is_gtest_matcher = void;

        private:
            Matcher<MatchesType> m_matcher;
        };

        template<typename MatchesType>
        Matcher<tempo_utils::Result<MatchesType>> ContainsResult(const Matcher<MatchesType> &matcher)
        {
            return ResultMatcher<MatchesType>(matcher);
        }

        template<typename MatchesType>
        class MaybeStatusMatcher {

        public:
            MaybeStatusMatcher(tempo_utils::StatusCode statusCode)
                : m_matcher(statusCode) {};
            MaybeStatusMatcher(std::string_view errorCategory)
                : m_matcher(errorCategory) {};
            MaybeStatusMatcher(std::string_view errorCategory, int errorCode)
                : m_matcher(errorCategory, errorCode) {};

            bool MatchAndExplain(const tempo_utils::MaybeStatus<MatchesType> &result, std::ostream *os) const {
                if (!result.isStatus())
                    return false;
                return m_matcher.MatchAndExplain(result.getStatus(), os);
            }
            void DescribeTo(std::ostream *os) const {
                m_matcher.DescribeTo(os);
            }
            void DescribeNegationTo(std::ostream *os) const {
                m_matcher.DescribeNegationTo(os);
            }

            using is_gtest_matcher = void;

        private:
            StatusMatcher m_matcher;
        };

        template<typename ConditionType,
            typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        Matcher<tempo_utils::MaybeStatus<tempo_utils::Status>> ContainsStatus(ConditionType condition)
        {
            return MaybeStatusMatcher<tempo_utils::Status>(
                tempo_utils::ConditionTraits<ConditionType>::condition_namespace(),
                static_cast<int>(condition));
        }
    }
}

namespace tempo_utils {

    template<class T>
    void PrintTo(const tempo_utils::Result<T> &result, std::ostream *os)
    {
        if (result.isResult()) {
            PrintTo(result.getResult(), os);
        } else {
            PrintTo(result.getStatus(), os);
        }
    }

    template<class T>
    void PrintTo(const tempo_utils::MaybeStatus<T> &maybeStatus, std::ostream *os)
    {
        if (maybeStatus.isStatus()) {
            PrintTo(maybeStatus.getStatus(), os);
        } else {
            *os << "no status";
        }
    }

    template<class T>
    std::ostream& operator<<(std::ostream& os, const Result<T> &result)
    {
        if (result.isResult()) {
            return os << result.getResult();
        } else {
            return os << result.getStatus();
        }
    }

    template<class T>
    std::ostream& operator<<(std::ostream& os, const MaybeStatus<T> &maybeStatus)
    {
        if (maybeStatus.isStatus()) {
            return os << maybeStatus.getStatus();
        } else {
            return os << "no status";
        }
    }
}

#endif // LYRIC_TEST_RESULT_MATCHERS_H