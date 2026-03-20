#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class BytesTests : public BaseBootstrapFixture {};

TEST_F(BytesTests, TestEvaluateBytesFromString)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.ToBytes()
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(MatchesDataCellType(lyric_runtime::DataCellType::BYTES))));
}

TEST_F(BytesTests, TestEvaluateBytesLength)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        val bytes: Bytes = string.ToBytes()
        bytes.Length()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(13))));
}

TEST_F(BytesTests, TestEvaluateBytesAt)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        val bytes: Bytes = string.ToBytes()
        bytes.At(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(72))));
}

TEST_F(BytesTests, TestEvaluateBytesAtInvalidIndex)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        val bytes: Bytes = string.ToBytes()
        bytes.At(100)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellUndef())));
}

TEST_F(BytesTests, TestEvaluateBytesEqual)
{
    auto result = runModule(R"(
        "hello".ToBytes() == "hello".ToBytes()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(BytesTests, TestEvaluateBytesLessThan)
{
    auto result = runModule(R"(
        "hello".ToBytes() < "goodbye".ToBytes()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(BytesTests, TestEvaluateBytesGreaterThan)
{
    auto result = runModule(R"(
        "hello".ToBytes() > "goodbye".ToBytes()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(BytesTests, TestEvaluateBytesLessOrEqual)
{
    auto result = runModule(R"(
        "hello".ToBytes() <= "goodbye".ToBytes()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(BytesTests, TestEvaluateBytesGreaterOrEqual)
{
    auto result = runModule(R"(
        "hello".ToBytes() >= "goodbye".ToBytes()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}
