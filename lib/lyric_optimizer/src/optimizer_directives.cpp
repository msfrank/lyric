
#include <lyric_optimizer/activation_state.h>
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
lyric_optimizer::Noop::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::Nil::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::Undef::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::Bool::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::Int::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::Float::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::Char::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::IntAdd::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::IntSub::getType() const
{
    return DirectiveType::IntSub;
}

tempo_utils::Status
lyric_optimizer::IntSub::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::IntSub::buildCode(
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
    return codeFragment->intSubtract();
}

std::string
lyric_optimizer::IntSub::toString() const
{
    auto lhs = m_lhs.isValid() ? m_lhs.resolveDirective()->toString() : "?";
    auto rhs = m_rhs.isValid() ? m_rhs.resolveDirective()->toString() : "?";
    return absl::StrCat("IntSub(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::IntMul::getType() const
{
    return DirectiveType::IntMul;
}

tempo_utils::Status
lyric_optimizer::IntMul::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::IntMul::buildCode(
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
    return codeFragment->intMultiply();
}

std::string
lyric_optimizer::IntMul::toString() const
{
    auto lhs = m_lhs.isValid() ? m_lhs.resolveDirective()->toString() : "?";
    auto rhs = m_rhs.isValid() ? m_rhs.resolveDirective()->toString() : "?";
    return absl::StrCat("IntMul(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::IntDiv::getType() const
{
    return DirectiveType::IntDiv;
}

tempo_utils::Status
lyric_optimizer::IntDiv::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::IntDiv::buildCode(
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
    return codeFragment->intDivide();
}

std::string
lyric_optimizer::IntDiv::toString() const
{
    auto lhs = m_lhs.isValid() ? m_lhs.resolveDirective()->toString() : "?";
    auto rhs = m_rhs.isValid() ? m_rhs.resolveDirective()->toString() : "?";
    return absl::StrCat("IntDiv(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::IntNeg::getType() const
{
    return DirectiveType::IntNeg;
}

tempo_utils::Status
lyric_optimizer::IntNeg::applyOperands(ActivationState &state, OperandStack &stack)
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

lyric_optimizer::LoadValue::LoadValue(const Instance &instance)
    : m_instance(instance)
{
    TU_ASSERT (m_instance.isValid());
}

lyric_optimizer::DirectiveType
lyric_optimizer::LoadValue::getType() const
{
    return DirectiveType::LoadValue;
}

tempo_utils::Status
lyric_optimizer::LoadValue::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::LoadValue::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return OptimizerStatus::forCondition(
        OptimizerCondition::kOptimizerInvariant, "unimplemented");
}

std::string
lyric_optimizer::LoadValue::toString() const
{
    auto name = m_instance.getName();
    return absl::StrCat("LoadValue(", name, ")");
}

lyric_optimizer::StoreValue::StoreValue(const Instance &instance)
    : m_instance(instance)
{
    TU_ASSERT (m_instance.isValid());
}

lyric_optimizer::DirectiveType
lyric_optimizer::StoreValue::getType() const
{
    return DirectiveType::StoreValue;
}

tempo_utils::Status
lyric_optimizer::StoreValue::applyOperands(ActivationState &state, OperandStack &stack)
{
    auto expression = stack.popOperand();
    if (!expression)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing store operand");
    TU_RETURN_IF_NOT_OK (m_instance.updateValue(expression));
    return {};
}

tempo_utils::Status
lyric_optimizer::StoreValue::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return OptimizerStatus::forCondition(
        OptimizerCondition::kOptimizerInvariant, "unimplemented");
}

std::string
lyric_optimizer::StoreValue::toString() const
{
    auto instance = m_instance.toString();
    auto value = m_instance.getValue();
    auto expression = value ? value->toString() : "?";
    return absl::StrCat("StoreValue ", instance);
}
