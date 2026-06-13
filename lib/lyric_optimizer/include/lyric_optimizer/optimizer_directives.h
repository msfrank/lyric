#ifndef LYRIC_OPTIMIZER_OPTIMIZER_DIRECTIVES_H
#define LYRIC_OPTIMIZER_OPTIMIZER_DIRECTIVES_H

#include <tempo_utils/integer_types.h>

#include "abstract_directive.h"
#include "directive_chain.h"
#include "instance.h"
#include "value.h"
#include "variable.h"

namespace lyric_optimizer {

    //
    class ExpressionDirective : public AbstractDirective {
    public:
        bool isExpression() const override;
    };

    //
    class StatementDirective : public AbstractDirective {
    public:
        bool isExpression() const override;
    };

    class Noop : public StatementDirective {
    public:
        DirectiveType getType() const override;
        bool isExpression() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    };

    class Nil : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    };

    class Undef : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    };

    class Bool : public ExpressionDirective {
    public:
        explicit Bool(bool b);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        bool m_b;
    };

    class I8 : public ExpressionDirective {
    public:
        explicit I8(tu_int8 i8);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        tu_int8 m_i8;
    };

    class I16 : public ExpressionDirective {
    public:
        explicit I16(tu_int16 i16);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        tu_int16 m_i16;
    };

    class I32 : public ExpressionDirective {
    public:
        explicit I32(tu_int32 i32);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        tu_int32 m_i32;
    };

    class I64 : public ExpressionDirective {
    public:
        explicit I64(tu_int64 i64);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        tu_int64 m_i64;
    };

    class U8 : public ExpressionDirective {
    public:
        explicit U8(tu_uint8 u8);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        tu_uint8 m_u8;
    };

    class U16 : public ExpressionDirective {
    public:
        explicit U16(tu_uint16 u16);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        tu_uint16 m_u16;
    };

    class U32 : public ExpressionDirective {
    public:
        explicit U32(tu_uint32 u32);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        tu_uint32 m_u32;
    };

    class U64 : public ExpressionDirective {
    public:
        explicit U64(tu_uint64 u64);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        tu_uint64 m_u64;
    };

    class F32 : public ExpressionDirective {
    public:
        explicit F32(float f32);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        float m_f32;
    };

    class F64 : public ExpressionDirective {
    public:
        explicit F64(double f64);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        double m_f64;
    };

    class C32 : public ExpressionDirective {
    public:
        explicit C32(char32_t c32);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        char32_t m_c32;
    };

    class Add : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        std::shared_ptr<AbstractDirective> m_lhs;
        std::shared_ptr<AbstractDirective> m_rhs;
    };

    class Sub : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        std::shared_ptr<AbstractDirective> m_lhs;
        std::shared_ptr<AbstractDirective> m_rhs;
    };

    class Mul : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        std::shared_ptr<AbstractDirective> m_lhs;
        std::shared_ptr<AbstractDirective> m_rhs;
    };

    class Div : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        std::shared_ptr<AbstractDirective> m_lhs;
        std::shared_ptr<AbstractDirective> m_rhs;
    };

    class Neg : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        std::shared_ptr<AbstractDirective> m_lhs;
    };

    class UseValue : public ExpressionDirective {
    public:
        explicit UseValue(const Instance &instance);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        Instance m_instance;
    };

    class DefineValue : public StatementDirective {
    public:
        explicit DefineValue(const Variable &variable);
        DirectiveType getType() const override;
        bool isEquivalentTo(std::shared_ptr<AbstractDirective> directive) const override;
        tempo_utils::Status applyOperands(ActivationState &state, OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
        Instance getInstance();
    private:
        Variable m_variable;
        Instance m_instance;
    };
}

#endif // LYRIC_OPTIMIZER_OPTIMIZER_DIRECTIVES_H
