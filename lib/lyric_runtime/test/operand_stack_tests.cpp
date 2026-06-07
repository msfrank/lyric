#include <stack>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_runtime/operand_stack.h>
#include <tempo_test/status_matchers.h>

class OperandStack : public ::testing::Test {};

TEST_F (OperandStack, PushAndPopOperand)
{
    lyric_runtime::OperandStack stack;

    tu_uint32 in = 42;
    auto value = lyric_runtime::Operand::fromU32(in);
    ASSERT_THAT (stack.pushOperand(value), tempo_test::IsOk());
    ASSERT_EQ (1, stack.getDepth());
    lyric_runtime::Operand top;
    ASSERT_THAT (stack.popOperand(top), tempo_test::IsOk());
    tu_uint32 out;
    ASSERT_TRUE (top.getU32(out));
    ASSERT_EQ (in, out);
    ASSERT_EQ (0, stack.getDepth());
}

TEST_F (OperandStack, PushAndPopMultipleOperands)
{
    lyric_runtime::OperandStack stack;
    const tu_uint32 count = 10;

    std::stack<tu_uint32> values;
    for (tu_uint32 i = 0; i < count; ++i) {
        auto value = lyric_runtime::Operand::fromU32(i);
        ASSERT_THAT (stack.pushOperand(value), tempo_test::IsOk());
        values.push(i);
    }
    ASSERT_EQ (count, stack.getDepth());

    for (tu_uint32 i = 0; i < count; ++i) {
        lyric_runtime::Operand top;
        ASSERT_THAT (stack.popOperand(top), tempo_test::IsOk());
        tu_uint32 in = values.top();
        values.pop();
        tu_uint32 out;
        ASSERT_TRUE (top.getU32(out));
        ASSERT_EQ (in, out);
    }

    ASSERT_EQ (0, stack.getDepth());
}

TEST_F (OperandStack, PushAndPeekOperand)
{
    lyric_runtime::OperandStack stack;

    tu_uint32 in = 42;
    auto value = lyric_runtime::Operand::fromU32(in);
    ASSERT_THAT (stack.pushOperand(value), tempo_test::IsOk());
    ASSERT_EQ (1, stack.getDepth());
    lyric_runtime::Operand top;
    ASSERT_THAT (stack.peekOperand(top), tempo_test::IsOk());
    tu_uint32 out;
    ASSERT_TRUE (top.getU32(out));
    ASSERT_EQ (in, out);
    ASSERT_EQ (1, stack.getDepth());
}

TEST_F (OperandStack, PushAndPeekMultipleOperands)
{
    lyric_runtime::OperandStack stack;
    const tu_uint32 count = 10;

    std::stack<tu_uint32> values;
    for (tu_uint32 i = 0; i < count; ++i) {
        auto value = lyric_runtime::Operand::fromU32(i);
        ASSERT_THAT (stack.pushOperand(value), tempo_test::IsOk());
        values.push(i);
    }
    ASSERT_EQ (count, stack.getDepth());

    for (tu_uint32 i = 0; i < count; ++i) {
        lyric_runtime::Operand top;
        ASSERT_THAT (stack.peekOperand(top, i), tempo_test::IsOk());
        tu_uint32 in = values.top();
        values.pop();
        tu_uint32 out;
        ASSERT_TRUE (top.getU32(out));
        ASSERT_EQ (in, out);
    }

    ASSERT_EQ (count, stack.getDepth());
}

TEST_F (OperandStack, PushAndDropOperand)
{
    lyric_runtime::OperandStack stack;

    auto value = lyric_runtime::Operand::nil();
    ASSERT_THAT (stack.pushOperand(value), tempo_test::IsOk());
    ASSERT_EQ (1, stack.getDepth());
    ASSERT_THAT (stack.dropOperand(), tempo_test::IsOk());
    ASSERT_EQ (0, stack.getDepth());
}

TEST_F (OperandStack, PushAndDropMultipleOperands)
{
    lyric_runtime::OperandStack stack;
    const tu_uint32 count = 10;

    for (tu_uint32 i = 0; i < count; ++i) {
        auto value = lyric_runtime::Operand::fromU32(i);
        ASSERT_THAT (stack.pushOperand(value), tempo_test::IsOk());
    }
    ASSERT_EQ (count, stack.getDepth());

    for (tu_uint32 i = 0; i < count; ++i) {
        ASSERT_THAT (stack.dropOperand(), tempo_test::IsOk());
    }

    ASSERT_EQ (0, stack.getDepth());
}
