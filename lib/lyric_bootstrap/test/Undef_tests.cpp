#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class UndefTests : public BaseBootstrapFixture {};

TEST_F(UndefTests, EvaluateUndef)
{
    auto result = runModule(R"(
        undef
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellUndef())));
}