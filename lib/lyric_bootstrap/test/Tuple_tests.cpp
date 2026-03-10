#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class TupleTests : public BaseBootstrapFixture {};

TEST_F(TupleTests, TestEvaluateTuple1)
{
    auto result = runModule(R"(
        val t: Tuple1[Int] = Tuple1[Int]{42}
        val i: Int = t.Element0
        i
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(TupleTests, TestEvaluateTuple2)
{
    auto result = runModule(R"(
        val t: Tuple2[Float,Int] = Tuple2[Float,Int]{0.1, 42}
        val i: Int = t.Element1
        i
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}

TEST_F(TupleTests, TestEvaluateTuple3)
{
    auto result = runModule(R"(
        val t: Tuple3[Float,String,Int] = Tuple3[Float,String,Int]{0.1, "hello, world!", 42}
        val i: Int = t.Element2
        i
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(42))));
}