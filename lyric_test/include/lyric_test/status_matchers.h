#ifndef LYRIC_TEST_STATUS_MATCHERS_H
#define LYRIC_TEST_STATUS_MATCHERS_H

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

        class StatusMatcher {

            enum class MatcherType {
                INVALID,
                ERROR_CATEGORY,
                ERROR_CATEGORY_AND_CODE,
                STATUS_CODE,
            };

        public:
            StatusMatcher();
            StatusMatcher(tempo_utils::StatusCode statusCode);
            StatusMatcher(std::string_view errorCategory);
            StatusMatcher(std::string_view errorCategory, int errorCode);
            StatusMatcher(const StatusMatcher &other);

            bool MatchAndExplain(const tempo_utils::Status &status, std::ostream *os) const;
            void DescribeTo(std::ostream *os) const;
            void DescribeNegationTo(std::ostream *os) const;

            using is_gtest_matcher = void;

        private:
            MatcherType m_type;
            tempo_utils::StatusCode m_statusCode;
            std::string m_errorCategory;
            int m_errorCode;
        };

        template<typename ConditionType,
            typename StatusType = typename tempo_utils::ConditionTraits<ConditionType>::StatusType>
        Matcher <tempo_utils::Status> IsCondition(ConditionType condition)
        {
            return StatusMatcher(
                tempo_utils::ConditionTraits<ConditionType>::condition_namespace(),
                static_cast<int>(condition));
        }

        Matcher <tempo_utils::Status> MatchesStatusCode(tempo_utils::StatusCode statusCode);
        Matcher <tempo_utils::Status> MatchesErrorCategory(const tempo_utils::SchemaNs &errorCategory);
        Matcher <tempo_utils::Status> MatchesErrorCategory(std::string_view errorCategory);
    }
}

namespace tempo_utils {
    void PrintTo(const tempo_utils::Status &status, std::ostream *os);
    std::ostream& operator<<(std::ostream& os, const Status &status);
}

#endif // LYRIC_TEST_STATUS_MATCHERS_H