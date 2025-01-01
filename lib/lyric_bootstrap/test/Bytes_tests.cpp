#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreBytes, TestEvaluateBytesFromString)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.ToBytes()
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(MatchesDataCellType(lyric_runtime::DataCellType::BYTES))));
}

TEST(CoreBytes, TestEvaluateBytesLength)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        val bytes: Bytes = string.ToBytes()
        bytes.Length();
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(13))));
}

TEST(CoreBytes, TestEvaluateBytesAt)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        val bytes: Bytes = string.ToBytes()
        bytes.At(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(72))));
}

TEST(CoreBytes, TestEvaluateBytesEqual)
{
    auto result = runModule(R"(
        val string1: String = "Hello, world!"
        val bytes1: Bytes = string1.ToBytes()
        val string2: String = "Hello, world!"
        val bytes2: Bytes = string2.ToBytes()
        bytes1 == bytes2
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}