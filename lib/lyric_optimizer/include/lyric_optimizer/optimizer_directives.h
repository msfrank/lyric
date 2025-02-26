#ifndef LYRIC_OPTIMIZER_OPTIMIZER_DIRECTIVES_H
#define LYRIC_OPTIMIZER_OPTIMIZER_DIRECTIVES_H

#include <tempo_utils/integer_types.h>
#include <unicode/umachine.h>

#include "abstract_directive.h"
#include "directive_chain.h"
#include "instance.h"
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
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    };

    class Nil : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    };

    class Undef : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    };

    class Bool : public ExpressionDirective {
    public:
        explicit Bool(bool b);
        DirectiveType getType() const override;
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        bool m_b;
    };

    class Int : public ExpressionDirective {
    public:
        explicit Int(tu_int64 i64);
        DirectiveType getType() const override;
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        tu_int64 m_i64;
    };

    class Float : public ExpressionDirective {
    public:
        explicit Float(double dbl);
        DirectiveType getType() const override;
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        double m_dbl;
    };

    class Char : public ExpressionDirective {
    public:
        explicit Char(UChar32 chr);
        DirectiveType getType() const override;
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        UChar32 m_chr;
    };

    class IntAdd : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        DirectiveChain m_lhs;
        DirectiveChain m_rhs;
    };

    // class IntSub : public ExpressionDirective {
    // public:
    //     IntSub(std::shared_ptr<DirectiveChain> lhs, std::shared_ptr<DirectiveChain> rhs);
    // private:
    //     std::shared_ptr<DirectiveChain> m_lhs;
    //     std::shared_ptr<DirectiveChain> m_rhs;
    // };
    //
    // class IntMul : public ExpressionDirective {
    // public:
    //     IntMul(std::shared_ptr<DirectiveChain> lhs, std::shared_ptr<DirectiveChain> rhs);
    // private:
    //     std::shared_ptr<DirectiveChain> m_lhs;
    //     std::shared_ptr<DirectiveChain> m_rhs;
    // };
    //
    // class IntDiv : public ExpressionDirective {
    // public:
    //     IntDiv(std::shared_ptr<DirectiveChain> lhs, std::shared_ptr<DirectiveChain> rhs);
    // private:
    //     std::shared_ptr<DirectiveChain> m_lhs;
    //     std::shared_ptr<DirectiveChain> m_rhs;
    // };

    class IntNeg : public ExpressionDirective {
    public:
        DirectiveType getType() const override;
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        DirectiveChain m_lhs;
    };

    class LoadLocal : public ExpressionDirective {
    public:
        explicit LoadLocal(const Variable &variable);
        DirectiveType getType() const override;
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        Variable m_variable;
    };

    class StoreLocal : public StatementDirective {
    public:
        explicit StoreLocal(const Variable &variable);
        DirectiveType getType() const override;
        tempo_utils::Status applyOperands(OperandStack &stack) override;
        tempo_utils::Status buildCode(
            lyric_assembler::CodeFragment *codeFragment,
            lyric_assembler::ProcHandle *procHandle) override;
        std::string toString() const override;
    private:
        Variable m_variable;
        DirectiveChain m_expression;
    };
}

#endif // LYRIC_OPTIMIZER_OPTIMIZER_DIRECTIVES_H
