
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

bool
lyric_optimizer::Noop::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    return other && other->getType() == DirectiveType::Noop;
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

bool
lyric_optimizer::Nil::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    return other && other->getType() == DirectiveType::Nil;
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

bool
lyric_optimizer::Undef::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    return other && other->getType() == DirectiveType::Undef;
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

bool
lyric_optimizer::Bool::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::Bool) {
        auto directive = std::static_pointer_cast<Bool>(other);
        return m_b == directive->m_b;
    }
    return false;
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

bool
lyric_optimizer::Int::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::Int) {
        auto directive = std::static_pointer_cast<Int>(other);
        return m_i64 == directive->m_i64;
    }
    return false;
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

bool
lyric_optimizer::Float::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::Float) {
        auto directive = std::static_pointer_cast<Float>(other);
        return m_dbl == directive->m_dbl;
    }
    return false;
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

lyric_optimizer::Char::Char(char32_t chr)
    : m_chr(chr)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::Char::getType() const
{
    return DirectiveType::Char;
}

bool
lyric_optimizer::Char::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::Char) {
        auto directive = std::static_pointer_cast<Char>(other);
        return m_chr == directive->m_chr;
    }
    return false;
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

bool
lyric_optimizer::IntAdd::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::IntAdd) {
        auto directive = std::static_pointer_cast<IntAdd>(other);
        return m_lhs->isEquivalentTo(directive->m_lhs)
            && m_rhs->isEquivalentTo(directive->m_rhs);
    }
    return false;
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
    m_rhs = std::move(rhs);
    m_lhs = std::move(lhs);
    return {};
}

tempo_utils::Status
lyric_optimizer::IntAdd::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    if (!m_lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    if (!m_rhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing rhs operand");
    TU_RETURN_IF_NOT_OK (m_lhs->buildCode(codeFragment, procHandle));
    TU_RETURN_IF_NOT_OK (m_rhs->buildCode(codeFragment, procHandle));
    return codeFragment->intAdd();
}

std::string
lyric_optimizer::IntAdd::toString() const
{
    auto lhs = m_lhs ? m_lhs->toString() : "?";
    auto rhs = m_rhs ? m_rhs->toString() : "?";
    return absl::StrCat("IntAdd(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::IntSub::getType() const
{
    return DirectiveType::IntSub;
}

bool
lyric_optimizer::IntSub::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::IntSub) {
        auto directive = std::static_pointer_cast<IntSub>(other);
        return m_lhs->isEquivalentTo(directive->m_lhs)
            && m_rhs->isEquivalentTo(directive->m_rhs);
    }
    return false;
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
    m_rhs = std::move(rhs);
    m_lhs = std::move(lhs);
    return {};
}

tempo_utils::Status
lyric_optimizer::IntSub::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    if (!m_lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    if (!m_rhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing rhs operand");
    TU_RETURN_IF_NOT_OK (m_lhs->buildCode(codeFragment, procHandle));
    TU_RETURN_IF_NOT_OK (m_rhs->buildCode(codeFragment, procHandle));
    return codeFragment->intSubtract();
}

std::string
lyric_optimizer::IntSub::toString() const
{
    auto lhs = m_lhs ? m_lhs->toString() : "?";
    auto rhs = m_rhs ? m_rhs->toString() : "?";
    return absl::StrCat("IntSub(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::IntMul::getType() const
{
    return DirectiveType::IntMul;
}

bool
lyric_optimizer::IntMul::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::IntMul) {
        auto directive = std::static_pointer_cast<IntMul>(other);
        return m_lhs->isEquivalentTo(directive->m_lhs)
            && m_rhs->isEquivalentTo(directive->m_rhs);
    }
    return false;
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
    m_rhs = std::move(rhs);
    m_lhs = std::move(lhs);
    return {};
}

tempo_utils::Status
lyric_optimizer::IntMul::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    if (!m_lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    if (!m_rhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing rhs operand");
    TU_RETURN_IF_NOT_OK (m_lhs->buildCode(codeFragment, procHandle));
    TU_RETURN_IF_NOT_OK (m_rhs->buildCode(codeFragment, procHandle));
    return codeFragment->intMultiply();
}

std::string
lyric_optimizer::IntMul::toString() const
{
    auto lhs = m_lhs ? m_lhs->toString() : "?";
    auto rhs = m_rhs ? m_rhs->toString() : "?";
    return absl::StrCat("IntMul(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::IntDiv::getType() const
{
    return DirectiveType::IntDiv;
}

bool
lyric_optimizer::IntDiv::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::IntDiv) {
        auto directive = std::static_pointer_cast<IntDiv>(other);
        return m_lhs->isEquivalentTo(directive->m_lhs)
            && m_rhs->isEquivalentTo(directive->m_rhs);
    }
    return false;
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
    m_rhs = std::move(rhs);
    m_lhs = std::move(lhs);
    return {};
}

tempo_utils::Status
lyric_optimizer::IntDiv::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    if (!m_lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    if (!m_rhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing rhs operand");
    TU_RETURN_IF_NOT_OK (m_lhs->buildCode(codeFragment, procHandle));
    TU_RETURN_IF_NOT_OK (m_rhs->buildCode(codeFragment, procHandle));
    return codeFragment->intDivide();
}

std::string
lyric_optimizer::IntDiv::toString() const
{
    auto lhs = m_lhs ? m_lhs->toString() : "?";
    auto rhs = m_rhs ? m_rhs->toString() : "?";
    return absl::StrCat("IntDiv(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::IntNeg::getType() const
{
    return DirectiveType::IntNeg;
}

bool
lyric_optimizer::IntNeg::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::IntNeg) {
        auto directive = std::static_pointer_cast<IntNeg>(other);
        return m_lhs->isEquivalentTo(directive->m_lhs);
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::IntNeg::applyOperands(ActivationState &state, OperandStack &stack)
{
    auto lhs = stack.popOperand();
    if (!lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    m_lhs = std::move(lhs);
    return {};
}

tempo_utils::Status
lyric_optimizer::IntNeg::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    if (!m_lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    TU_RETURN_IF_NOT_OK (m_lhs->buildCode(codeFragment, procHandle));
    return codeFragment->intNegate();
}

std::string
lyric_optimizer::IntNeg::toString() const
{
    auto lhs = m_lhs ? m_lhs->toString() : "?";
    return absl::StrCat("IntNeg(", lhs, ")");
}

lyric_optimizer::UseValue::UseValue(const Instance &instance)
    : m_instance(instance)
{
    //TU_ASSERT (m_instance.isValid());
}

lyric_optimizer::DirectiveType
lyric_optimizer::UseValue::getType() const
{
    return DirectiveType::UseValue;
}

bool
lyric_optimizer::UseValue::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::UseValue) {
        auto directive = std::static_pointer_cast<UseValue>(other);
        return m_instance.isEquivalentTo(directive->m_instance);
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::UseValue::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::UseValue::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return OptimizerStatus::forCondition(
        OptimizerCondition::kOptimizerInvariant, "unimplemented");
}

std::string
lyric_optimizer::UseValue::toString() const
{
    return absl::StrCat("UseValue(", m_instance.getName(), ")");
}

lyric_optimizer::DefineValue::DefineValue(const Variable &variable)
    : m_variable(variable)
{
    TU_ASSERT (m_variable.isValid());
}

lyric_optimizer::DirectiveType
lyric_optimizer::DefineValue::getType() const
{
    return DirectiveType::DefineValue;
}

bool
lyric_optimizer::DefineValue::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::DefineValue) {
        auto directive = std::static_pointer_cast<DefineValue>(other);
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::DefineValue::applyOperands(ActivationState &state, OperandStack &stack)
{
    auto expression = stack.popOperand();
    if (!expression)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing value");
    Value value(expression);
    TU_ASSIGN_OR_RETURN (m_instance, m_variable.makeInstance());
    TU_RETURN_IF_NOT_OK (m_instance.setValue(value));
    TU_RETURN_IF_NOT_OK (state.mutateVariable(m_variable, m_instance));
    return {};
}

tempo_utils::Status
lyric_optimizer::DefineValue::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return {};
}

std::string
lyric_optimizer::DefineValue::toString() const
{
    auto instance = m_instance.toString();
    return absl::StrCat("DefineValue ", instance);
}
