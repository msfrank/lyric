#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class RestTests : public BaseBootstrapFixture {};

TEST_F(RestTests, TestEvaluateRestSize)
{
    auto result = runModule(R"(
        def CountArgs(args: ...I64): I64 {
            args.Size()
        }
        CountArgs(1, 2, 3, 4, 5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandInt(5))));
}

TEST_F(RestTests, TestEvaluateRestGet)
{
    auto result = runModule(R"(
        def ArgAt(index: I64, args: ...I64): I64 | Nil {
            args.Get(index)
        }
        ArgAt(0, 1, 2, 3, 4, 5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandInt(1))));
}

TEST_F(RestTests, TestEvaluateRestIterateImpl)
{
    auto result = runModule(R"(
        def SumArgs(args: ...I64): I64 {
            var count: I64 = 0
            for n: I64 in args {
                count += n
            }
            count
        }
        SumArgs(1, 2, 3, 4, 5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(OperandInt(15))));
}
