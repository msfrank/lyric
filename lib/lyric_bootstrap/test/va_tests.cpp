#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class VaTests : public BaseBootstrapFixture {};

TEST_F(VaTests, EvaluateVaSize)
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

TEST_F(VaTests, EvaluateVaLoad)
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
