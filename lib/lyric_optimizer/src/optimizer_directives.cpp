
#include <lyric_optimizer/instance.h>
#include <lyric_optimizer/operand_stack.h>
#include <lyric_optimizer/optimizer_directives.h>
#include <lyric_optimizer/optimizer_result.h>
#include <tempo_utils/unicode.h>

bool
lyric_optimizer::ExpressionDirective::isExpression() const
{
    return true;
}

bool
lyric_optimizer::StatementDirective::isExpression() const
{
    return false;
}

lyric_optimizer::DirectiveType
lyric_optimizer::Noop::getType() const
{
    return DirectiveType::Noop;
}

bool
lyric_optimizer::Noop::isExpression() const
{
    return false;
}

tempo_utils::Status
lyric_optimizer::Noop::applyOperands(OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::Noop::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->noOperation();
}

std::string
lyric_optimizer::Noop::toString() const
{
    return "[Noop]";
}

lyric_optimizer::DirectiveType
lyric_optimizer::Nil::getType() const
{
    return DirectiveType::Nil;
}

tempo_utils::Status
lyric_optimizer::Nil::applyOperands(OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::Nil::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateNil();
}

std::string
lyric_optimizer::Nil::toString() const
{
    return "Nil()";
}

lyric_optimizer::DirectiveType
lyric_optimizer::Undef::getType() const
{
    return DirectiveType::Undef;
}

tempo_utils::Status
lyric_optimizer::Undef::applyOperands(OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::Undef::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateUndef();
}

std::string
lyric_optimizer::Undef::toString() const
{
    return "Undef()";
}

lyric_optimizer::Bool::Bool(bool b)
    : m_b(b)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::Bool::getType() const
{
    return DirectiveType::Bool;
}

tempo_utils::Status
lyric_optimizer::Bool::applyOperands(OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::Bool::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateBool(m_b);
}

std::string
lyric_optimizer::Bool::toString() const
{
    return absl::StrCat("Bool(", m_b, ")");
}

lyric_optimizer::Int::Int(tu_int64 i64)
    : m_i64(i64)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::Int::getType() const
{
    return DirectiveType::Int;
}

tempo_utils::Status
lyric_optimizer::Int::applyOperands(OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::Int::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateInt(m_i64);
}

std::string
lyric_optimizer::Int::toString() const
{
    return absl::StrCat("Int(", m_i64, ")");
}

lyric_optimizer::Float::Float(double dbl)
    : m_dbl(dbl)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::Float::getType() const
{
    return DirectiveType::Float;
}

tempo_utils::Status
lyric_optimizer::Float::applyOperands(OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::Float::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateFloat(m_dbl);
}

std::string
lyric_optimizer::Float::toString() const
{
    return absl::StrCat("Float(", m_dbl, ")");
}

lyric_optimizer::Char::Char(UChar32 chr)
    : m_chr(chr)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::Char::getType() const
{
    return DirectiveType::Char;
}

tempo_utils::Status
lyric_optimizer::Char::applyOperands(OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::Char::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateChar(m_chr);
}

std::string
lyric_optimizer::Char::toString() const
{
    return absl::StrCat("Char(", tempo_utils::convert_to_utf8(m_chr), ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::IntAdd::getType() const
{
    return DirectiveType::IntAdd;
}

tempo_utils::Status
lyric_optimizer::IntAdd::applyOperands(OperandStack &stack)
{
    auto rhs = stack.popOperand();
    if (!rhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    auto lhs = stack.popOperand();
    if (!lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    m_rhs.forwardDirective(rhs);
    m_lhs.forwardDirective(lhs);
    return {};
}

tempo_utils::Status
lyric_optimizer::IntAdd::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    auto lhs = m_lhs.resolveDirective();
    if (!lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    auto rhs = m_rhs.resolveDirective();
    if (!rhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing rhs operand");
    TU_RETURN_IF_NOT_OK (lhs->buildCode(codeFragment, procHandle));
    TU_RETURN_IF_NOT_OK (rhs->buildCode(codeFragment, procHandle));
    return codeFragment->intAdd();
}

std::string
lyric_optimizer::IntAdd::toString() const
{
    auto lhs = m_lhs.isValid() ? m_lhs.resolveDirective()->toString() : "?";
    auto rhs = m_rhs.isValid() ? m_rhs.resolveDirective()->toString() : "?";
    return absl::StrCat("IntAdd(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::IntNeg::getType() const
{
    return DirectiveType::IntNeg;
}

tempo_utils::Status
lyric_optimizer::IntNeg::applyOperands(OperandStack &stack)
{
    auto lhs = stack.popOperand();
    if (!lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    m_lhs.forwardDirective(lhs);
    return {};
}

tempo_utils::Status
lyric_optimizer::IntNeg::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    auto lhs = m_lhs.resolveDirective();
    if (!lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    TU_RETURN_IF_NOT_OK (lhs->buildCode(codeFragment, procHandle));
    return codeFragment->intNegate();
}

std::string
lyric_optimizer::IntNeg::toString() const
{
    auto lhs = m_lhs.isValid() ? m_lhs.resolveDirective()->toString() : "?";
    return absl::StrCat("IntNeg(", lhs, ")");
}

lyric_optimizer::LoadLocal::LoadLocal(const Variable &variable)
    : m_variable(variable)
{
    TU_ASSERT (m_variable.isValid());
}

lyric_optimizer::DirectiveType
lyric_optimizer::LoadLocal::getType() const
{
    return DirectiveType::LoadLocal;
}

tempo_utils::Status
lyric_optimizer::LoadLocal::applyOperands(OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::LoadLocal::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    //return codeFragment->loadData();
    return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant, "unimplemented");
}

std::string
lyric_optimizer::LoadLocal::toString() const
{
    auto variable = m_variable.toString();
    return absl::StrCat("LoadLocal(", variable, ")");
}

lyric_optimizer::StoreLocal::StoreLocal(const Variable &variable)
    : m_variable(variable)
{
    TU_ASSERT (m_variable.isValid());
}

lyric_optimizer::DirectiveType
lyric_optimizer::StoreLocal::getType() const
{
    return DirectiveType::StoreLocal;
}

tempo_utils::Status
lyric_optimizer::StoreLocal::applyOperands(OperandStack &stack)
{
    auto expression = stack.popOperand();
    if (!expression)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing store expression operand");
    m_expression.forwardDirective(expression);
    return {};
}

tempo_utils::Status
lyric_optimizer::StoreLocal::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    //return codeFragment->storeData();
    return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant, "unimplemented");
}

std::string
lyric_optimizer::StoreLocal::toString() const
{
    auto variable = m_variable.toString();
    auto expression = m_expression.isValid() ? m_expression.resolveDirective()->toString() : "?";
    return absl::StrCat("[StoreLocal ", variable, ", ", expression, "]");
}
