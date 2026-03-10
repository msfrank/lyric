#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class OptionTests : public BaseBootstrapFixture {};

TEST_F(OptionTests, TestNewEmptyOptionUsingUnionType)
{
    auto result = runModule(R"(
        val opt: Bool|Nil = nil
        opt
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellNil())));
}