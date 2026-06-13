#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_runtime/internal/convert_ops.h>
#include <lyric_runtime/operand.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/memory_bytes.h>

class ConvertOps : public ::testing::Test {};

TEST_F (ConvertOps, ConvertI8ToU8)
{
    auto from = lyric_runtime::Operand::fromI8(100);
    lyric_runtime::Operand result;
    ASSERT_THAT (lyric_runtime::internal::convert_to_U8(from, result), tempo_test::IsOk());
    tu_uint8 u8;
    ASSERT_TRUE (result.getU8(u8));
    ASSERT_EQ (100, u8);
}

