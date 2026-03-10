#include <gtest/gtest.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class SeqTests : public BaseBootstrapFixture {};

TEST_F(SeqTests, TestEvaluateNewSeq)
{
    auto result = runModule(R"(
        val seq: Seq = Seq{1, 2, 3}
        seq
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_bootstrap::preludeSymbol("Seq")))));
}

TEST_F(SeqTests, TestEvaluateSeqSize)
{
    auto result = runModule(R"(
        val seq: Seq = Seq{1, 2, 3}
        seq.Size()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(SeqTests, TestEvaluateSeqGet)
{
    auto result = runModule(R"(
        val seq: Seq = Seq{1, 2, 3}
        seq.GetOrElse(0, 0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST_F(SeqTests, TestEvaluateSeqAppend)
{
    auto result = runModule(R"(
        val seq1: Seq = Seq{1, 2, 3}
        val seq2: Seq = seq1.Append(4, 5, 6)
        seq2.GetOrElse(5, 0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(6))));
}

TEST_F(SeqTests, TestEvaluateSeqExtend)
{
    auto result = runModule(R"(
        val seq1: Seq = Seq{1, 2, 3}
        val seq2: Seq = Seq{4, 5, 6}
        val seq3: Seq = seq1.Extend(seq2)
        seq3.GetOrElse(5, 0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(6))));
}

TEST_F(SeqTests, TestEvaluateSeqSlice)
{
    auto result = runModule(R"(
        val seq1: Seq = Seq{1, 2, 3, 4, 5, 6}
        val seq2: Seq = seq1.Slice(2, 2)
        seq2.GetOrElse(1, 0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(4))));
}

TEST_F(SeqTests, TestEvaluateSeqIterateImpl)
{
    auto result = runModule(R"(
        val seq: Seq = Seq{1, 2, 3, 4, 5, 6}
        var count: Int = 0
        for n: Any in seq {
            set count += 1
        }
        count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(6))));
}
