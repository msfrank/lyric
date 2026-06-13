
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

lyric_optimizer::I8::I8(tu_int8 i8)
    : m_i8(i8)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::I8::getType() const
{
    return DirectiveType::I8;
}

bool
lyric_optimizer::I8::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::I8) {
        auto directive = std::static_pointer_cast<I8>(other);
        return m_i8 == directive->m_i8;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::I8::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::I8::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateI8(m_i8);
}

std::string
lyric_optimizer::I8::toString() const
{
    return absl::StrCat("I8(", m_i8, ")");
}

lyric_optimizer::I16::I16(tu_int16 i16)
    : m_i16(i16)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::I16::getType() const
{
    return DirectiveType::I16;
}

bool
lyric_optimizer::I16::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::I16) {
        auto directive = std::static_pointer_cast<I16>(other);
        return m_i16 == directive->m_i16;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::I16::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::I16::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateI16(m_i16);
}

std::string
lyric_optimizer::I16::toString() const
{
    return absl::StrCat("I16(", m_i16, ")");
}

lyric_optimizer::I32::I32(tu_int32 i32)
    : m_i32(i32)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::I32::getType() const
{
    return DirectiveType::I32;
}

bool
lyric_optimizer::I32::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::I32) {
        auto directive = std::static_pointer_cast<I32>(other);
        return m_i32 == directive->m_i32;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::I32::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::I32::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateI32(m_i32);
}

std::string
lyric_optimizer::I32::toString() const
{
    return absl::StrCat("I32(", m_i32, ")");
}

lyric_optimizer::I64::I64(tu_int64 i64)
    : m_i64(i64)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::I64::getType() const
{
    return DirectiveType::I64;
}

bool
lyric_optimizer::I64::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::I64) {
        auto directive = std::static_pointer_cast<I64>(other);
        return m_i64 == directive->m_i64;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::I64::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::I64::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateI64(m_i64);
}

std::string
lyric_optimizer::I64::toString() const
{
    return absl::StrCat("I64(", m_i64, ")");
}

lyric_optimizer::U8::U8(tu_uint8 u8)
    : m_u8(u8)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::U8::getType() const
{
    return DirectiveType::U8;
}

bool
lyric_optimizer::U8::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::U8) {
        auto directive = std::static_pointer_cast<U8>(other);
        return m_u8 == directive->m_u8;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::U8::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::U8::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateU8(m_u8);
}

std::string
lyric_optimizer::U8::toString() const
{
    return absl::StrCat("U8(", m_u8, ")");
}

lyric_optimizer::U16::U16(tu_uint16 u16)
    : m_u16(u16)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::U16::getType() const
{
    return DirectiveType::U16;
}

bool
lyric_optimizer::U16::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::U16) {
        auto directive = std::static_pointer_cast<U16>(other);
        return m_u16 == directive->m_u16;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::U16::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::U16::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateU16(m_u16);
}

std::string
lyric_optimizer::U16::toString() const
{
    return absl::StrCat("U16(", m_u16, ")");
}

lyric_optimizer::U32::U32(tu_uint32 u32)
    : m_u32(u32)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::U32::getType() const
{
    return DirectiveType::U32;
}

bool
lyric_optimizer::U32::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::U32) {
        auto directive = std::static_pointer_cast<U32>(other);
        return m_u32 == directive->m_u32;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::U32::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::U32::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateU32(m_u32);
}

std::string
lyric_optimizer::U32::toString() const
{
    return absl::StrCat("U32(", m_u32, ")");
}

lyric_optimizer::U64::U64(tu_uint64 u64)
    : m_u64(u64)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::U64::getType() const
{
    return DirectiveType::U64;
}

bool
lyric_optimizer::U64::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::U64) {
        auto directive = std::static_pointer_cast<U64>(other);
        return m_u64 == directive->m_u64;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::U64::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::U64::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateU64(m_u64);
}

std::string
lyric_optimizer::U64::toString() const
{
    return absl::StrCat("U64(", m_u64, ")");
}

lyric_optimizer::F32::F32(float f32)
    : m_f32(f32)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::F32::getType() const
{
    return DirectiveType::F32;
}

bool
lyric_optimizer::F32::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::F32) {
        auto directive = std::static_pointer_cast<F32>(other);
        return m_f32 == directive->m_f32;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::F32::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::F32::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateF32(m_f32);
}

std::string
lyric_optimizer::F32::toString() const
{
    return absl::StrCat("F32(", m_f32, ")");
}

lyric_optimizer::F64::F64(double f64)
    : m_f64(f64)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::F64::getType() const
{
    return DirectiveType::F64;
}

bool
lyric_optimizer::F64::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::F64) {
        auto directive = std::static_pointer_cast<F64>(other);
        return m_f64 == directive->m_f64;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::F64::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::F64::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateF64(m_f64);
}

std::string
lyric_optimizer::F64::toString() const
{
    return absl::StrCat("F64(", m_f64, ")");
}

lyric_optimizer::C32::C32(char32_t c32)
    : m_c32(c32)
{
}

lyric_optimizer::DirectiveType
lyric_optimizer::C32::getType() const
{
    return DirectiveType::C32;
}

bool
lyric_optimizer::C32::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::C32) {
        auto directive = std::static_pointer_cast<C32>(other);
        return m_c32 == directive->m_c32;
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::C32::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::C32::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return codeFragment->immediateC32(m_c32);
}

std::string
lyric_optimizer::C32::toString() const
{
    return absl::StrCat("C32(", tempo_utils::convert_to_utf8(m_c32), ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::Add::getType() const
{
    return DirectiveType::Add;
}

bool
lyric_optimizer::Add::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::Add) {
        auto directive = std::static_pointer_cast<Add>(other);
        return m_lhs->isEquivalentTo(directive->m_lhs)
            && m_rhs->isEquivalentTo(directive->m_rhs);
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::Add::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::Add::buildCode(
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
    return codeFragment->add();
}

std::string
lyric_optimizer::Add::toString() const
{
    auto lhs = m_lhs ? m_lhs->toString() : "?";
    auto rhs = m_rhs ? m_rhs->toString() : "?";
    return absl::StrCat("Add(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::Sub::getType() const
{
    return DirectiveType::Sub;
}

bool
lyric_optimizer::Sub::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::Sub) {
        auto directive = std::static_pointer_cast<Sub>(other);
        return m_lhs->isEquivalentTo(directive->m_lhs)
            && m_rhs->isEquivalentTo(directive->m_rhs);
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::Sub::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::Sub::buildCode(
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
    return codeFragment->subtract();
}

std::string
lyric_optimizer::Sub::toString() const
{
    auto lhs = m_lhs ? m_lhs->toString() : "?";
    auto rhs = m_rhs ? m_rhs->toString() : "?";
    return absl::StrCat("Sub(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::Mul::getType() const
{
    return DirectiveType::Mul;
}

bool
lyric_optimizer::Mul::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::Mul) {
        auto directive = std::static_pointer_cast<Mul>(other);
        return m_lhs->isEquivalentTo(directive->m_lhs)
            && m_rhs->isEquivalentTo(directive->m_rhs);
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::Mul::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::Mul::buildCode(
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
    return codeFragment->multiply();
}

std::string
lyric_optimizer::Mul::toString() const
{
    auto lhs = m_lhs ? m_lhs->toString() : "?";
    auto rhs = m_rhs ? m_rhs->toString() : "?";
    return absl::StrCat("Mul(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::Div::getType() const
{
    return DirectiveType::Div;
}

bool
lyric_optimizer::Div::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::Div) {
        auto directive = std::static_pointer_cast<Div>(other);
        return m_lhs->isEquivalentTo(directive->m_lhs)
            && m_rhs->isEquivalentTo(directive->m_rhs);
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::Div::applyOperands(ActivationState &state, OperandStack &stack)
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
lyric_optimizer::Div::buildCode(
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
    return codeFragment->divide();
}

std::string
lyric_optimizer::Div::toString() const
{
    auto lhs = m_lhs ? m_lhs->toString() : "?";
    auto rhs = m_rhs ? m_rhs->toString() : "?";
    return absl::StrCat("Div(", lhs, ", ", rhs, ")");
}

lyric_optimizer::DirectiveType
lyric_optimizer::Neg::getType() const
{
    return DirectiveType::Neg;
}

bool
lyric_optimizer::Neg::isEquivalentTo(std::shared_ptr<AbstractDirective> other) const
{
    if (other && other->getType() == DirectiveType::Neg) {
        auto directive = std::static_pointer_cast<Neg>(other);
        return m_lhs->isEquivalentTo(directive->m_lhs);
    }
    return false;
}

tempo_utils::Status
lyric_optimizer::Neg::applyOperands(ActivationState &state, OperandStack &stack)
{
    auto lhs = stack.popOperand();
    if (!lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    m_lhs = std::move(lhs);
    return {};
}

tempo_utils::Status
lyric_optimizer::Neg::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    if (!m_lhs)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lhs operand");
    TU_RETURN_IF_NOT_OK (m_lhs->buildCode(codeFragment, procHandle));
    return codeFragment->negate();
}

std::string
lyric_optimizer::Neg::toString() const
{
    auto lhs = m_lhs ? m_lhs->toString() : "?";
    return absl::StrCat("Neg(", lhs, ")");
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
