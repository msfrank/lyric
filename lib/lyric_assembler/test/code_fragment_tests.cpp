#include <gtest/gtest.h>

#include <lyric_assembler/code_fragment.h>
#include <tempo_test/status_matchers.h>

TEST(CodeFragment, ImmediateNil)
{
    auto activation = lyric_common::SymbolUrl::fromString("#sym");
    lyric_assembler::ProcHandle proc(activation);
    lyric_assembler::CodeFragment code(&proc);

    ASSERT_THAT (code.immediateNil(), tempo_test::IsOk());

    lyric_object::BytecodeBuilder bytecodeBuilder;
    ASSERT_THAT (code.write(bytecodeBuilder), tempo_test::IsOk());

    auto bytecode = bytecodeBuilder.getBytecode();
    lyric_object::BytecodeIterator it(bytecode.data(), bytecode.size());
    lyric_object::OpCell op;

    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_NIL, op.opcode);
    ASSERT_EQ (lyric_object::OpInfoType::NO_OPERANDS, op.type);
    ASSERT_FALSE (it.hasNext());
}

TEST(CodeFragment, UnconditionalJump)
{
    auto activation = lyric_common::SymbolUrl::fromString("#sym");
    lyric_assembler::ProcHandle proc(activation);
    lyric_assembler::CodeFragment code(&proc);

    code.appendLabel("top");
    code.noOperation();
    tu_uint32 targetId;
    TU_ASSIGN_OR_RAISE (targetId, code.unconditionalJump());
    code.patchTarget(targetId, "top");

    lyric_object::BytecodeBuilder bytecodeBuilder;
    ASSERT_THAT (code.write(bytecodeBuilder), tempo_test::IsOk());

    auto bytecode = bytecodeBuilder.getBytecode();
    lyric_object::BytecodeIterator it(bytecode.data(), bytecode.size());
    lyric_object::OpCell op;

    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_NOOP, op.opcode);
    ASSERT_EQ (lyric_object::OpInfoType::NO_OPERANDS, op.type);

    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_JUMP, op.opcode);
    ASSERT_EQ (lyric_object::OpInfoType::JUMP_I16, op.type);
    ASSERT_EQ (-4, op.operands.jump_i16.jump);

    ASSERT_FALSE (it.hasNext());
}
