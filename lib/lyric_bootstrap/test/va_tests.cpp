#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreVa, EvaluateVaSize)
{
    auto result = runModule(R"(
        def CountVaArgs(...Any): Int {
            VaSize()
        }
        CountVaArgs(1, 2, 3)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(3))));
}

TEST(CoreVa, EvaluateVaLoad)
{
    auto result = runModule(R"(
        def GetVaArg(index: Int, ...Any): Any {
            VaLoad(index)
        }
        GetVaArg(1, 1, 2, 3)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(2))));
}
