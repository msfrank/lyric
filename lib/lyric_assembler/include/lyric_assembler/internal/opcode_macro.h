#ifndef LYRIC_ASSEMBLER_INTERNAL_OPCODE_MACRO_H
#define LYRIC_ASSEMBLER_INTERNAL_OPCODE_MACRO_H

#include <lyric_object/object_types.h>
#include <lyric_rewriter/abstract_macro.h>

namespace lyric_assembler::internal {

    class NoOperandOpcodeMacro : public lyric_rewriter::AbstractMacro {
    public:
        explicit NoOperandOpcodeMacro(lyric_object::Opcode opcode);

        tempo_utils::Status rewritePragma(
            const lyric_parser::ArchetypeNode *pragmaNode,
            lyric_rewriter::PragmaContext &ctx,
            lyric_parser::ArchetypeState *state) override;

        tempo_utils::Status rewriteDefinition(
            const lyric_parser::ArchetypeNode *macroCallNode,
            lyric_parser::ArchetypeNode *definitionNode,
            lyric_parser::ArchetypeState *state) override;

        tempo_utils::Status rewriteBlock(
            const lyric_parser::ArchetypeNode *macroCallNode,
            lyric_rewriter::MacroBlock &macroBlock,
            lyric_parser::ArchetypeState *state) override;

    private:
        lyric_object::Opcode m_opcode;
    };

    class StackOffsetOpcodeMacro : public lyric_rewriter::AbstractMacro {
    public:
        explicit StackOffsetOpcodeMacro(lyric_object::Opcode opcode);

        tempo_utils::Status rewritePragma(
            const lyric_parser::ArchetypeNode *pragmaNode,
            lyric_rewriter::PragmaContext &ctx,
            lyric_parser::ArchetypeState *state) override;

        tempo_utils::Status rewriteDefinition(
            const lyric_parser::ArchetypeNode *macroCallNode,
            lyric_parser::ArchetypeNode *definitionNode,
            lyric_parser::ArchetypeState *state) override;

        tempo_utils::Status rewriteBlock(
            const lyric_parser::ArchetypeNode *macroCallNode,
            lyric_rewriter::MacroBlock &macroBlock,
            lyric_parser::ArchetypeState *state) override;

    private:
        lyric_object::Opcode m_opcode;
    };

    class NoopMacro : public NoOperandOpcodeMacro {
    public:
        NoopMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_NOOP) {};
    };

    // stack manipulation macros

    class PopMacro : public NoOperandOpcodeMacro {
    public:
        PopMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_POP) {};
    };

    class DupMacro : public NoOperandOpcodeMacro {
    public:
        DupMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_DUP) {};
    };

    class PickMacro : public StackOffsetOpcodeMacro {
    public:
        PickMacro() : StackOffsetOpcodeMacro(lyric_object::Opcode::OP_PICK) {};
    };

    class DropMacro : public StackOffsetOpcodeMacro {
    public:
        DropMacro() : StackOffsetOpcodeMacro(lyric_object::Opcode::OP_DROP) {};
    };

    class RPickMacro : public StackOffsetOpcodeMacro {
    public:
        RPickMacro() : StackOffsetOpcodeMacro(lyric_object::Opcode::OP_RPICK) {};
    };

    class RDropMacro : public StackOffsetOpcodeMacro {
    public:
        RDropMacro() : StackOffsetOpcodeMacro(lyric_object::Opcode::OP_RDROP) {};
    };

    // integer math macros

    class I64AddMacro : public NoOperandOpcodeMacro {
    public:
        I64AddMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_I64_ADD) {};
    };

    class I64SubMacro : public NoOperandOpcodeMacro {
    public:
        I64SubMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_I64_SUB) {};
    };

    class I64MulMacro : public NoOperandOpcodeMacro {
    public:
        I64MulMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_I64_MUL) {};
    };

    class I64DivMacro : public NoOperandOpcodeMacro {
    public:
        I64DivMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_I64_DIV) {};
    };

    class I64NegMacro : public NoOperandOpcodeMacro {
    public:
        I64NegMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_I64_NEG) {};
    };

    // rational math macros

    class DblAddMacro : public NoOperandOpcodeMacro {
    public:
        DblAddMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_DBL_ADD) {};
    };

    class DblSubMacro : public NoOperandOpcodeMacro {
    public:
        DblSubMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_DBL_SUB) {};
    };

    class DblMulMacro : public NoOperandOpcodeMacro {
    public:
        DblMulMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_DBL_MUL) {};
    };

    class DblDivMacro : public NoOperandOpcodeMacro {
    public:
        DblDivMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_DBL_DIV) {};
    };

    class DblNegMacro : public NoOperandOpcodeMacro {
    public:
        DblNegMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_DBL_NEG) {};
    };

    // comparison macros

    class I64CmpMacro : public NoOperandOpcodeMacro {
    public:
        I64CmpMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_I64_CMP) {};
    };

    class DblCmpMacro : public NoOperandOpcodeMacro {
    public:
        DblCmpMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_DBL_CMP) {};
    };

    class BoolCmpMacro : public NoOperandOpcodeMacro {
    public:
        BoolCmpMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_BOOL_CMP) {};
    };

    class ChrCmpMacro : public NoOperandOpcodeMacro {
    public:
        ChrCmpMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_CHR_CMP) {};
    };

    class TypeCmpMacro : public NoOperandOpcodeMacro {
    public:
        TypeCmpMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_TYPE_CMP) {};
    };

    // logical operation macros

    class LogicalAndMacro : public NoOperandOpcodeMacro {
    public:
        LogicalAndMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_LOGICAL_AND) {};
    };

    class LogicalOrMacro : public NoOperandOpcodeMacro {
    public:
        LogicalOrMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_LOGICAL_OR) {};
    };

    class LogicalNotMacro : public NoOperandOpcodeMacro {
    public:
        LogicalNotMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_LOGICAL_NOT) {};
    };

    // bitwise operation macros

    class BitwiseAndMacro : public NoOperandOpcodeMacro {
    public:
        BitwiseAndMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_BITWISE_AND) {};
    };

    class BitwiseOrMacro : public NoOperandOpcodeMacro {
    public:
        BitwiseOrMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_BITWISE_OR) {};
    };

    class BitwiseXorMacro : public NoOperandOpcodeMacro {
    public:
        BitwiseXorMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_BITWISE_XOR) {};
    };

    class BitwiseLeftShiftMacro : public NoOperandOpcodeMacro {
    public:
        BitwiseLeftShiftMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_BITWISE_LEFT_SHIFT) {};
    };

    class BitwiseRightShiftMacro : public NoOperandOpcodeMacro {
    public:
        BitwiseRightShiftMacro() : NoOperandOpcodeMacro(lyric_object::Opcode::OP_BITWISE_RIGHT_SHIFT) {};
    };
}

#endif // LYRIC_ASSEMBLER_INTERNAL_OPCODE_MACRO_H