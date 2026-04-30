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

TEST_F(BytesTests, TestEvaluateBytesAppend)
{
    auto result = runModule(R"(
        val bytes1 = "Hello, ".ToBytes()
        val bytes2 = "world!".ToBytes()
        bytes1.Append(bytes2)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellBytes("Hello, world!"))));
}

TEST_F(BytesTests, TestEvaluateBytesPrepend)
{
    auto result = runModule(R"(
        val bytes1 = "Hello, ".ToBytes()
        val bytes2 = "world!".ToBytes()
        bytes2.Prepend(bytes1)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellBytes("Hello, world!"))));
}

TEST_F(BytesTests, TestEvaluateBytesInsert)
{
    auto result = runModule(R"(
        val bytes1 = "Helloworld!".ToBytes()
        val bytes2 = ", ".ToBytes()
        bytes1.Insert(5, bytes2)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellBytes("Hello, world!"))));
}

TEST_F(BytesTests, TestEvaluateBytesRemove)
{
    auto result = runModule(R"(
        val bytes1 = "Hello, world!".ToBytes()
        bytes1.Remove(5, 2)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellBytes("Helloworld!"))));
}

TEST_F(BytesTests, TestEvaluateBytesSubspan)
{
    auto result = runModule(R"(
        val bytes1 = "Helloworld!".ToBytes()
        bytes1.Subspan(5, 5)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellBytes("world"))));
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
