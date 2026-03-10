#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class NilTests : public BaseBootstrapFixture {};

TEST_F(NilTests, EvaluateNil)
{
    auto result = runModule(R"(
        nil
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellNil())));
}