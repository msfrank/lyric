#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class TupleTests : public BaseBootstrapFixture {};

TEST_F(TupleTests, TestEvaluateTuple1)
{
    auto result = runModule(R"(
        val t: Tuple1[I64] = Tuple1[I64]{42}
        val i: I64 = t.Element0
        i
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(42))));
}

TEST_F(TupleTests, TestEvaluateTuple2)
{
    auto result = runModule(R"(
        val t: Tuple2[F64,I64] = Tuple2[F64,I64]{0.1, 42}
        val i: I64 = t.Element1
        i
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(42))));
}

TEST_F(TupleTests, TestEvaluateTuple3)
{
    auto result = runModule(R"(
        val t: Tuple3[F64,String,I64] = Tuple3[F64,String,I64]{0.1, "hello, world!", 42}
        val i: I64 = t.Element2
        i
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(42))));
}