#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_runtime/tagged_value.h>
#include <tempo_utils/memory_bytes.h>

#include "runtime_mocks.h"

class TaggedValue : public ::testing::Test {};

TEST_F (TaggedValue, RoundtripUndef)
{
    auto value = lyric_runtime::TaggedValue::undef();
    ASSERT_EQ (lyric_runtime::DataCellType::Undef, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    ASSERT_TRUE (value.isUndef());
}

TEST_F (TaggedValue, RoundtripNil)
{
    auto value = lyric_runtime::TaggedValue::nil();
    ASSERT_EQ (lyric_runtime::DataCellType::Nil, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    ASSERT_TRUE (value.isNil());
}

TEST_F (TaggedValue, RoundtripTrue)
{
    bool in = true;
    auto value = lyric_runtime::TaggedValue::fromBool(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Bool, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    bool out;
    ASSERT_TRUE (value.getBool(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripFalse)
{
    bool in = false;
    auto value = lyric_runtime::TaggedValue::fromBool(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Bool, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    bool out;
    ASSERT_TRUE (value.getBool(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripU8)
{
    tu_uint8 in = 178;
    auto value = lyric_runtime::TaggedValue::fromU8(in);
    ASSERT_EQ (lyric_runtime::DataCellType::UInt8, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    tu_uint8 out;
    ASSERT_TRUE (value.getU8(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripPositiveI8)
{
    tu_int8 in = 42;
    auto value = lyric_runtime::TaggedValue::fromI8(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int8, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    tu_int8 out;
    ASSERT_TRUE (value.getI8(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripNegativeI8)
{
    tu_int8 in = -42;
    auto value = lyric_runtime::TaggedValue::fromI8(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int8, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    tu_int8 out;
    ASSERT_TRUE (value.getI8(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripU16)
{
    tu_uint16 in = 15030;
    auto value = lyric_runtime::TaggedValue::fromU16(in);
    ASSERT_EQ (lyric_runtime::DataCellType::UInt16, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    tu_uint16 out;
    ASSERT_TRUE (value.getU16(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripPositiveI16)
{
    tu_int16 in = 25030;
    auto value = lyric_runtime::TaggedValue::fromI16(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int16, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    tu_int16 out;
    ASSERT_TRUE (value.getI16(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripNegativeI16)
{
    tu_int16 in = -25030;
    auto value = lyric_runtime::TaggedValue::fromI16(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int16, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    tu_int16 out;
    ASSERT_TRUE (value.getI16(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripSmallU32)
{
    tu_uint32 in = 100000;
    auto value = lyric_runtime::TaggedValue::fromU32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::UInt32, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    tu_uint32 out;
    ASSERT_TRUE (value.getU32(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripLargeU32)
{
    tu_uint32 in = 3000000000;
    auto value = lyric_runtime::TaggedValue::fromU32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::UInt32, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::LargeValue, value.getRepresentation());
    tu_uint32 out;
    ASSERT_TRUE (value.getU32(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripSmallPositiveI32)
{
    tu_int32 in = 1000;
    auto value = lyric_runtime::TaggedValue::fromI32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int32, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    tu_int32 out;
    ASSERT_TRUE (value.getI32(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripLargePositiveI32)
{
    tu_int32 in = 2000000000;
    auto value = lyric_runtime::TaggedValue::fromI32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int32, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::LargeValue, value.getRepresentation());
    tu_int32 out;
    ASSERT_TRUE (value.getI32(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripSmallNegativeI32)
{
    tu_int32 in = -1000;
    auto value = lyric_runtime::TaggedValue::fromI32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int32, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    tu_int32 out;
    ASSERT_TRUE (value.getI32(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripLargeNegativeI32)
{
    tu_int32 in = -2000000000;
    auto value = lyric_runtime::TaggedValue::fromI32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int32, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::LargeValue, value.getRepresentation());
    tu_int32 out;
    ASSERT_TRUE (value.getI32(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripU64)
{
    tu_uint64 in = 3000000000000;
    auto value = lyric_runtime::TaggedValue::fromU64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::UInt64, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::LargeValue, value.getRepresentation());
    tu_uint64 out;
    ASSERT_TRUE (value.getU64(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, ConversionFromU64FailsWhenOutOfRange)
{
    tu_uint64 in = std::numeric_limits<tu_uint64>::max();
    auto value = lyric_runtime::TaggedValue::fromU64(in);
    ASSERT_FALSE (value.isValid());
    ASSERT_EQ (lyric_runtime::DataCellType::Invalid, value.getType());
}

TEST_F (TaggedValue, RoundtripPositiveI64)
{
    tu_int64 in = 500000000000;
    auto value = lyric_runtime::TaggedValue::fromI64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int64, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::LargeValue, value.getRepresentation());
    tu_int64 out;
    ASSERT_TRUE (value.getI64(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripNegativeI64)
{
    tu_int64 in = -500000000000;
    auto value = lyric_runtime::TaggedValue::fromI64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Int64, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::LargeValue, value.getRepresentation());
    tu_int64 out;
    ASSERT_TRUE (value.getI64(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, ConversionFromI64FailsWhenTooLarge)
{
    tu_int64 in = std::numeric_limits<tu_int64>::max();
    auto value = lyric_runtime::TaggedValue::fromI64(in);
    ASSERT_FALSE (value.isValid());
    ASSERT_EQ (lyric_runtime::DataCellType::Invalid, value.getType());
}

TEST_F (TaggedValue, ConversionFromI64FailsWhenTooSmall)
{
    tu_int64 in = std::numeric_limits<tu_int64>::min();
    auto value = lyric_runtime::TaggedValue::fromI64(in);
    ASSERT_FALSE (value.isValid());
    ASSERT_EQ (lyric_runtime::DataCellType::Invalid, value.getType());
}

TEST_F (TaggedValue, RoundtripChar32)
{
    char32_t in = U'人';
    auto value = lyric_runtime::TaggedValue::fromC32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Char32, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    char32_t out;
    ASSERT_TRUE (value.getC32(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, ConversionFromC32FailsWhenOutOfRange)
{
    auto value = lyric_runtime::TaggedValue::fromC32(std::numeric_limits<char32_t>::max());
    ASSERT_FALSE (value.isValid());
    ASSERT_EQ (lyric_runtime::DataCellType::Invalid, value.getType());
}

TEST_F (TaggedValue, RoundtripFloat32)
{
    float in = 3.14159;
    auto value = lyric_runtime::TaggedValue::fromF32(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Float32, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::LargeValue, value.getRepresentation());
    float out;
    ASSERT_TRUE (value.getF32(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripFloat64)
{
    double in = 7.0;
    auto value = lyric_runtime::TaggedValue::fromF64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Float64, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::LargeValue, value.getRepresentation());
    double out;
    ASSERT_TRUE (value.getF64(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripFloat64PositiveZero)
{
    double in = 0.0;
    auto value = lyric_runtime::TaggedValue::fromF64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Float64, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    double out;
    ASSERT_TRUE (value.getF64(out));
    ASSERT_EQ (in, out);
}

TEST_F (TaggedValue, RoundtripFloat64NegativeZero)
{
    double in = -0.0;
    auto value = lyric_runtime::TaggedValue::fromF64(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Float64, value.getType());
    ASSERT_EQ (lyric_runtime::TaggedRepresentation::SmallValue, value.getRepresentation());
    double out;
    ASSERT_TRUE (value.getF64(out));
    ASSERT_EQ (in, out);
}