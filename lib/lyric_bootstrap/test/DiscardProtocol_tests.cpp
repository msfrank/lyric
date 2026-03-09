#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreDiscardProtocol, EvaluateDiscardProtocol)
{
    GTEST_SKIP();
    auto result = runModule(R"(
        DiscardProtocol
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellNil())));
}
