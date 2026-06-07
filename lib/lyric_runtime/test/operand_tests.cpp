#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_runtime/operand.h>
#include <tempo_utils/memory_bytes.h>

class Operand : public ::testing::Test {};

TEST_F (Operand, DefaultConstructedIsInvalid)
{
    lyric_runtime::Operand value;
    ASSERT_EQ (lyric_runtime::DataCellType::Invalid, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::Invalid, value.getOverlay());
    ASSERT_FALSE (value.isValid());
}

TEST_F (Operand, RoundtripUndef)
{
    auto value = lyric_runtime::Operand::undef();
    ASSERT_EQ (lyric_runtime::DataCellType::Undef, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    ASSERT_TRUE (value.isUndef());
}

TEST_F (Operand, RoundtripNil)
{
    auto value = lyric_runtime::Operand::nil();
    ASSERT_EQ (lyric_runtime::DataCellType::Nil, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    ASSERT_TRUE (value.isNil());
}

TEST_F (Operand, RoundtripTrue)
{
    bool in = true;
    auto value = lyric_runtime::Operand::fromBool(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Bool, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    bool out;
    ASSERT_TRUE (value.getBool(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripFalse)
{
    bool in = false;
    auto value = lyric_runtime::Operand::fromBool(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Bool, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    bool out;
    ASSERT_TRUE (value.getBool(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripU8)
{
    tu_uint8 in = 178;
    auto value = lyric_runtime::Operand::fromU8(in);
    ASSERT_EQ (lyric_runtime::DataCellType::UInt8, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    tu_uint8 out;
    ASSERT_TRUE (value.getU8(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripPositiveI8)
{
    tu_int8 in = 42;
    auto value = lyric_runtime::Operand::fromI8(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int8, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    tu_int8 out;
    ASSERT_TRUE (value.getI8(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripNegativeI8)
{
    tu_int8 in = -42;
    auto value = lyric_runtime::Operand::fromI8(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int8, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    tu_int8 out;
    ASSERT_TRUE (value.getI8(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripU16)
{
    tu_uint16 in = 15030;
    auto value = lyric_runtime::Operand::fromU16(in);
    ASSERT_EQ (lyric_runtime::DataCellType::UInt16, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    tu_uint16 out;
    ASSERT_TRUE (value.getU16(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripPositiveI16)
{
    tu_int16 in = 25030;
    auto value = lyric_runtime::Operand::fromI16(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int16, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    tu_int16 out;
    ASSERT_TRUE (value.getI16(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripNegativeI16)
{
    tu_int16 in = -25030;
    auto value = lyric_runtime::Operand::fromI16(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int16, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    tu_int16 out;
    ASSERT_TRUE (value.getI16(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripSmallU32)
{
    tu_uint32 in = 100000;
    auto value = lyric_runtime::Operand::fromU32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::UInt32, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    tu_uint32 out;
    ASSERT_TRUE (value.getU32(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripLargeU32)
{
    tu_uint32 in = 3000000000;
    auto value = lyric_runtime::Operand::fromU32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::UInt32, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::LargeValue, value.getOverlay());
    tu_uint32 out;
    ASSERT_TRUE (value.getU32(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripSmallPositiveI32)
{
    tu_int32 in = 1000;
    auto value = lyric_runtime::Operand::fromI32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int32, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    tu_int32 out;
    ASSERT_TRUE (value.getI32(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripLargePositiveI32)
{
    tu_int32 in = 2000000000;
    auto value = lyric_runtime::Operand::fromI32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int32, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::LargeValue, value.getOverlay());
    tu_int32 out;
    ASSERT_TRUE (value.getI32(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripSmallNegativeI32)
{
    tu_int32 in = -1000;
    auto value = lyric_runtime::Operand::fromI32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int32, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    tu_int32 out;
    ASSERT_TRUE (value.getI32(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripLargeNegativeI32)
{
    tu_int32 in = -2000000000;
    auto value = lyric_runtime::Operand::fromI32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int32, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::LargeValue, value.getOverlay());
    tu_int32 out;
    ASSERT_TRUE (value.getI32(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripU64)
{
    tu_uint64 in = 3000000000000;
    auto value = lyric_runtime::Operand::fromU64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::UInt64, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::LargeValue, value.getOverlay());
    tu_uint64 out;
    ASSERT_TRUE (value.getU64(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, ConversionFromU64FailsWhenOutOfRange)
{
    tu_uint64 in = std::numeric_limits<tu_uint64>::max();
    auto value = lyric_runtime::Operand::fromU64(in);
    ASSERT_FALSE (value.isValid());
    ASSERT_EQ (lyric_runtime::DataCellType::Invalid, value.getType());
}

TEST_F (Operand, RoundtripPositiveI64)
{
    tu_int64 in = 500000000000;
    auto value = lyric_runtime::Operand::fromI64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int64, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::LargeValue, value.getOverlay());
    tu_int64 out;
    ASSERT_TRUE (value.getI64(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripNegativeI64)
{
    tu_int64 in = -500000000000;
    auto value = lyric_runtime::Operand::fromI64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int64, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::LargeValue, value.getOverlay());
    tu_int64 out;
    ASSERT_TRUE (value.getI64(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, ConversionFromI64FailsWhenTooLarge)
{
    tu_int64 in = std::numeric_limits<tu_int64>::max();
    auto value = lyric_runtime::Operand::fromI64(in);
    ASSERT_FALSE (value.isValid());
    ASSERT_EQ (lyric_runtime::DataCellType::Invalid, value.getType());
}

TEST_F (Operand, ConversionFromI64FailsWhenTooSmall)
{
    tu_int64 in = std::numeric_limits<tu_int64>::min();
    auto value = lyric_runtime::Operand::fromI64(in);
    ASSERT_FALSE (value.isValid());
    ASSERT_EQ (lyric_runtime::DataCellType::Invalid, value.getType());
}

TEST_F (Operand, RoundtripChar32)
{
    char32_t in = U'人';
    auto value = lyric_runtime::Operand::fromC32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Char32, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    char32_t out;
    ASSERT_TRUE (value.getC32(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, ConversionFromC32FailsWhenOutOfRange)
{
    auto value = lyric_runtime::Operand::fromC32(std::numeric_limits<char32_t>::max());
    ASSERT_FALSE (value.isValid());
    ASSERT_EQ (lyric_runtime::DataCellType::Invalid, value.getType());
}

TEST_F (Operand, RoundtripFloat32)
{
    float in = 3.14159;
    auto value = lyric_runtime::Operand::fromF32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Float32, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::LargeValue, value.getOverlay());
    float out;
    ASSERT_TRUE (value.getF32(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripFloat64)
{
    double in = 7.0;
    auto value = lyric_runtime::Operand::fromF64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Float64, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::LargeValue, value.getOverlay());
    double out;
    ASSERT_TRUE (value.getF64(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripFloat64PositiveZero)
{
    double in = 0.0;
    auto value = lyric_runtime::Operand::fromF64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Float64, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    double out;
    ASSERT_TRUE (value.getF64(out));
    ASSERT_EQ (in, out);
}

TEST_F (Operand, RoundtripFloat64NegativeZero)
{
    double in = -0.0;
    auto value = lyric_runtime::Operand::fromF64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Float64, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::SmallValue, value.getOverlay());
    double out;
    ASSERT_TRUE (value.getF64(out));
    ASSERT_EQ (in, out);
}