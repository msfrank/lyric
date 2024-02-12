#ifndef LYRIC_TEST_TEST_RESULT_H
#define LYRIC_TEST_TEST_RESULT_H

#include <string>

#include <fmt/core.h>
#include <fmt/format.h>

#include <tempo_utils/status.h>

namespace lyric_test {

    constexpr tempo_utils::SchemaNs kLyricTestStatusNs("dev.zuri.ns:lyric-test-status-1");

    enum class TestCondition {
        kInvalidConfiguration,
        kTestInvariant,
    };


    class TestStatus : public tempo_utils::TypedStatus<TestCondition> {
    public:
        using TypedStatus::TypedStatus;
        static TestStatus ok();
        static bool convert(TestStatus &dstStatus, const tempo_utils::Status &srcStatus);

    private:
        TestStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @tparam Args
         * @param condition
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename... Args>
        static TestStatus forCondition(
            TestCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return TestStatus(condition, message);
        }
        /**
         *
         * @tparam Args
         * @param condition
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename... Args>
        static TestStatus forCondition(
            TestCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return TestStatus(condition, message, traceId, spanId);
        }
    };

    class TestException : public std::exception {
    public:
        TestException(const TestStatus &status) noexcept;
        TestStatus getStatus() const;
        const char* what() const noexcept override;

    private:
        TestStatus m_status;
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_test::TestCondition> {
        using ConditionType = lyric_test::TestCondition;
        static bool convert(lyric_test::TestStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_test::TestStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_test::TestCondition> {
        using StatusType = lyric_test::TestStatus;
        static constexpr const char *condition_namespace() { return lyric_test::kLyricTestStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_test::TestCondition condition)
        {
            switch (condition) {
                case lyric_test::TestCondition::kInvalidConfiguration:
                case lyric_test::TestCondition::kTestInvariant:
                    return tempo_utils::StatusCode::kInternal;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_test::TestCondition condition)
        {
            switch (condition) {
                case lyric_test::TestCondition::kInvalidConfiguration:
                    return "Invalid configuration";
                case lyric_test::TestCondition::kTestInvariant:
                    return "Test invariant";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_TEST_TEST_RESULT_H