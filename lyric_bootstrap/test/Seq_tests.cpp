#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreSeq, TestEvaluateNewSeq)
{
    auto result = runModule(R"(
        val seq: Seq = Seq{1, 2, 3}
        seq
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("Seq"))))));
}

TEST(CoreSeq, TestEvaluateSeqSize)
{
    auto result = runModule(R"(
        val seq: Seq = Seq{1, 2, 3}
        seq.Size()
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}

TEST(CoreSeq, TestEvaluateSeqGet)
{
    auto result = runModule(R"(
        val seq: Seq = Seq{1, 2, 3}
        seq.GetOrElse(0, 0)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(1)))));
}

TEST(CoreSeq, TestEvaluateSeqAppend)
{
    auto result = runModule(R"(
        val seq1: Seq = Seq{1, 2, 3}
        val seq2: Seq = seq1.Append(4, 5, 6)
        seq2.GetOrElse(5, 0)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(6)))));
}

TEST(CoreSeq, TestEvaluateSeqExtend)
{
    auto result = runModule(R"(
        val seq1: Seq = Seq{1, 2, 3}
        val seq2: Seq = Seq{4, 5, 6}
        val seq3: Seq = seq1.Extend(seq2)
        seq3.GetOrElse(5, 0)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(6)))));
}

TEST(CoreSeq, TestEvaluateSeqSlice)
{
    auto result = runModule(R"(
        val seq1: Seq = Seq{1, 2, 3, 4, 5, 6}
        val seq2: Seq = seq1.Slice(2, 2)
        seq2.GetOrElse(1, 0)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(4)))));
}

TEST(CoreSeq, TestEvaluateSeqIter)
{
    auto result = runModule(R"(
        val seq: Seq = Seq{1, 2, 3, 4, 5, 6}
        var count: Int = 0
        for n in seq.Iter() {
            set count += 1
        }
        count
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(6)))));
}