#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_common/parse_numeric.h>
#include <tempo_test/tempo_test.h>

class ParseNumeric : public ::testing::Test {};

TEST_F (ParseNumeric, DecimalI64)
{
    auto result = lyric_common::parse_I64("12345");
    ASSERT_THAT (result, tempo_test::IsResult());
    auto value = result.getResult();
    ASSERT_EQ (12345, value);
}

TEST_F (ParseNumeric, DecimalI64Negative)
{
    auto result = lyric_common::parse_I64("-12345");
    ASSERT_THAT (result, tempo_test::IsResult());
    auto value = result.getResult();
    ASSERT_EQ (-12345, value);
}

TEST_F (ParseNumeric, HexI64)
{
    auto result = lyric_common::parse_I64("0xFF2A");
    ASSERT_THAT (result, tempo_test::IsResult());
    auto value = result.getResult();
    ASSERT_EQ (65322, value);
}

TEST_F (ParseNumeric, HexI64Negative)
{
    auto result = lyric_common::parse_I64("-0x12BB");
    ASSERT_THAT (result, tempo_test::IsResult());
    auto value = result.getResult();
    ASSERT_EQ (-4795, value);
}

TEST_F (ParseNumeric, DecimalF64)
{
    auto result = lyric_common::parse_F64("0.12345");
    ASSERT_THAT (result, tempo_test::IsResult());
    auto value = result.getResult();
    ASSERT_DOUBLE_EQ (0.12345, value);
}

TEST_F (ParseNumeric, DecimalF32)
{
    auto result = lyric_common::parse_F32("0.12345");
    ASSERT_THAT (result, tempo_test::IsResult());
    auto value = result.getResult();
    ASSERT_FLOAT_EQ (0.12345, value);
}
