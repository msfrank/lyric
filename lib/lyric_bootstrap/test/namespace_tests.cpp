#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreNamespace, EvaluateDeclareNamespace)
{
    auto result = runModule(R"(
        namespace foo {
            def bar(): Int { 42 }
        }
        foo.bar()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}