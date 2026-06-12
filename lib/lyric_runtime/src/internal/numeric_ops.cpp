
#include <boost/safe_numerics/checked_default.hpp>

#include <lyric_runtime/internal/numeric_ops.h>
#include <lyric_runtime/internal/operand_helpers.h>
#include <lyric_runtime/interpreter_result.h>

template<class ValueType>
tempo_utils::Status apply_add(
    const lyric_runtime::Operand &lhs,
    const lyric_runtime::Operand &rhs,
    lyric_runtime::Operand &result)
{
    ValueType l, r;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_binary_operands(lhs, rhs, l, r));
    auto check = boost::safe_numerics::checked::add(l, r);
    if (check.exception()) [[unlikely]] {
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, static_cast<const char *>(check));
    }
    value_to_operand(static_cast<ValueType>(check), result);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_sub(
    const lyric_runtime::Operand &lhs,
    const lyric_runtime::Operand &rhs,
    lyric_runtime::Operand &result)
{
    ValueType l, r;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_binary_operands(lhs, rhs, l, r));
    auto check = boost::safe_numerics::checked::subtract(l, r);
    if (check.exception()) [[unlikely]] {
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, static_cast<const char *>(check));
    }
    value_to_operand(static_cast<ValueType>(check), result);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_mul(
    const lyric_runtime::Operand &lhs,
    const lyric_runtime::Operand &rhs,
    lyric_runtime::Operand &result)
{
    ValueType l, r;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_binary_operands(lhs, rhs, l, r));
    auto check = boost::safe_numerics::checked::multiply(l, r);
    if (check.exception()) [[unlikely]] {
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, static_cast<const char *>(check));
    }
    value_to_operand(static_cast<ValueType>(check), result);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_div(
    const lyric_runtime::Operand &lhs,
    const lyric_runtime::Operand &rhs,
    lyric_runtime::Operand &result)
{
    ValueType l, r;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_binary_operands(lhs, rhs, l, r));
    auto check = boost::safe_numerics::checked::divide(l, r);
    if (check.exception()) [[unlikely]] {
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, static_cast<const char *>(check));
    }
    value_to_operand(static_cast<ValueType>(check), result);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_neg(
    const lyric_runtime::Operand &element,
    lyric_runtime::Operand &result)
{
    ValueType e;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    value_to_operand(-e, result);
    return {};
}

tempo_utils::Status
lyric_runtime::internal::add(const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8:    return apply_add<tu_uint8>(lhs, rhs, result);
        case OperandType::UInt16:   return apply_add<tu_uint16>(lhs, rhs, result);
        case OperandType::UInt32:   return apply_add<tu_uint32>(lhs, rhs, result);
        case OperandType::UInt64:   return apply_add<tu_uint64>(lhs, rhs, result);
        case OperandType::Int8:     return apply_add<tu_int8>(lhs, rhs, result);
        case OperandType::Int16:    return apply_add<tu_int16>(lhs, rhs, result);
        case OperandType::Int32:    return apply_add<tu_int32>(lhs, rhs, result);
        case OperandType::Int64:    return apply_add<tu_int64>(lhs, rhs, result);
        case OperandType::Float32:  return apply_add<float>(lhs, rhs, result);
        case OperandType::Float64:  return apply_add<double>(lhs, rhs, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::sub(const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8:    return apply_sub<tu_uint8>(lhs, rhs, result);
        case OperandType::UInt16:   return apply_sub<tu_uint16>(lhs, rhs, result);
        case OperandType::UInt32:   return apply_sub<tu_uint32>(lhs, rhs, result);
        case OperandType::UInt64:   return apply_sub<tu_uint64>(lhs, rhs, result);
        case OperandType::Int8:     return apply_sub<tu_int8>(lhs, rhs, result);
        case OperandType::Int16:    return apply_sub<tu_int16>(lhs, rhs, result);
        case OperandType::Int32:    return apply_sub<tu_int32>(lhs, rhs, result);
        case OperandType::Int64:    return apply_sub<tu_int64>(lhs, rhs, result);
        case OperandType::Float32:  return apply_sub<float>(lhs, rhs, result);
        case OperandType::Float64:  return apply_sub<double>(lhs, rhs, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::mul(const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8:    return apply_mul<tu_uint8>(lhs, rhs, result);
        case OperandType::UInt16:   return apply_mul<tu_uint16>(lhs, rhs, result);
        case OperandType::UInt32:   return apply_mul<tu_uint32>(lhs, rhs, result);
        case OperandType::UInt64:   return apply_mul<tu_uint64>(lhs, rhs, result);
        case OperandType::Int8:     return apply_mul<tu_int8>(lhs, rhs, result);
        case OperandType::Int16:    return apply_mul<tu_int16>(lhs, rhs, result);
        case OperandType::Int32:    return apply_mul<tu_int32>(lhs, rhs, result);
        case OperandType::Int64:    return apply_mul<tu_int64>(lhs, rhs, result);
        case OperandType::Float32:  return apply_mul<float>(lhs, rhs, result);
        case OperandType::Float64:  return apply_mul<double>(lhs, rhs, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::div(const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8:    return apply_div<tu_uint8>(lhs, rhs, result);
        case OperandType::UInt16:   return apply_div<tu_uint16>(lhs, rhs, result);
        case OperandType::UInt32:   return apply_div<tu_uint32>(lhs, rhs, result);
        case OperandType::UInt64:   return apply_div<tu_uint64>(lhs, rhs, result);
        case OperandType::Int8:     return apply_div<tu_int8>(lhs, rhs, result);
        case OperandType::Int16:    return apply_div<tu_int16>(lhs, rhs, result);
        case OperandType::Int32:    return apply_div<tu_int32>(lhs, rhs, result);
        case OperandType::Int64:    return apply_div<tu_int64>(lhs, rhs, result);
        case OperandType::Float32:  return apply_div<float>(lhs, rhs, result);
        case OperandType::Float64:  return apply_div<double>(lhs, rhs, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::neg(const Operand &element, Operand &result)
{
    switch (element.getType()) {
        case OperandType::UInt8:    return apply_neg<tu_uint8>(element, result);
        case OperandType::UInt16:   return apply_neg<tu_uint16>(element, result);
        case OperandType::UInt32:   return apply_neg<tu_uint32>(element, result);
        case OperandType::UInt64:   return apply_neg<tu_uint64>(element, result);
        case OperandType::Int8:     return apply_neg<tu_int8>(element, result);
        case OperandType::Int16:    return apply_neg<tu_int16>(element, result);
        case OperandType::Int32:    return apply_neg<tu_int32>(element, result);
        case OperandType::Int64:    return apply_neg<tu_int64>(element, result);
        case OperandType::Float32:  return apply_neg<float>(element, result);
        case OperandType::Float64:  return apply_neg<double>(element, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}