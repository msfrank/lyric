
#include <boost/safe_numerics/checked_default.hpp>

#include <lyric_runtime/internal/numeric_ops.h>
#include <lyric_runtime/internal/operand_helpers.h>
#include <lyric_runtime/interpreter_result.h>

template<class ValueType>
tempo_utils::Status apply_add(
    lyric_runtime::HeapManager *heapManager,
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
    value_to_operand(static_cast<ValueType>(check), result, heapManager);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_sub(
    lyric_runtime::HeapManager *heapManager,
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
    value_to_operand(static_cast<ValueType>(check), result, heapManager);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_mul(
    lyric_runtime::HeapManager *heapManager,
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
    value_to_operand(static_cast<ValueType>(check), result, heapManager);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_div(
    lyric_runtime::HeapManager *heapManager,
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
    value_to_operand(static_cast<ValueType>(check), result, heapManager);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_neg(
    lyric_runtime::HeapManager *heapManager,
    const lyric_runtime::Operand &element,
    lyric_runtime::Operand &result)
{
    ValueType e;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    value_to_operand(-e, result, heapManager);
    return {};
}

tempo_utils::Status
lyric_runtime::internal::add(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8:    return apply_add<tu_uint8>(heapManager, lhs, rhs, result);
        case OperandType::UInt16:   return apply_add<tu_uint16>(heapManager, lhs, rhs, result);
        case OperandType::UInt32:   return apply_add<tu_uint32>(heapManager, lhs, rhs, result);
        case OperandType::UInt64:   return apply_add<tu_uint64>(heapManager, lhs, rhs, result);
        case OperandType::Int8:     return apply_add<tu_int8>(heapManager, lhs, rhs, result);
        case OperandType::Int16:    return apply_add<tu_int16>(heapManager, lhs, rhs, result);
        case OperandType::Int32:    return apply_add<tu_int32>(heapManager, lhs, rhs, result);
        case OperandType::Int64:    return apply_add<tu_int64>(heapManager, lhs, rhs, result);
        case OperandType::Float32:  return apply_add<float>(heapManager, lhs, rhs, result);
        case OperandType::Float64:  return apply_add<double>(heapManager, lhs, rhs, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::sub(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8:    return apply_sub<tu_uint8>(heapManager, lhs, rhs, result);
        case OperandType::UInt16:   return apply_sub<tu_uint16>(heapManager, lhs, rhs, result);
        case OperandType::UInt32:   return apply_sub<tu_uint32>(heapManager, lhs, rhs, result);
        case OperandType::UInt64:   return apply_sub<tu_uint64>(heapManager, lhs, rhs, result);
        case OperandType::Int8:     return apply_sub<tu_int8>(heapManager, lhs, rhs, result);
        case OperandType::Int16:    return apply_sub<tu_int16>(heapManager, lhs, rhs, result);
        case OperandType::Int32:    return apply_sub<tu_int32>(heapManager, lhs, rhs, result);
        case OperandType::Int64:    return apply_sub<tu_int64>(heapManager, lhs, rhs, result);
        case OperandType::Float32:  return apply_sub<float>(heapManager, lhs, rhs, result);
        case OperandType::Float64:  return apply_sub<double>(heapManager, lhs, rhs, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::mul(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8:    return apply_mul<tu_uint8>(heapManager, lhs, rhs, result);
        case OperandType::UInt16:   return apply_mul<tu_uint16>(heapManager, lhs, rhs, result);
        case OperandType::UInt32:   return apply_mul<tu_uint32>(heapManager, lhs, rhs, result);
        case OperandType::UInt64:   return apply_mul<tu_uint64>(heapManager, lhs, rhs, result);
        case OperandType::Int8:     return apply_mul<tu_int8>(heapManager, lhs, rhs, result);
        case OperandType::Int16:    return apply_mul<tu_int16>(heapManager, lhs, rhs, result);
        case OperandType::Int32:    return apply_mul<tu_int32>(heapManager, lhs, rhs, result);
        case OperandType::Int64:    return apply_mul<tu_int64>(heapManager, lhs, rhs, result);
        case OperandType::Float32:  return apply_mul<float>(heapManager, lhs, rhs, result);
        case OperandType::Float64:  return apply_mul<double>(heapManager, lhs, rhs, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::div(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8:    return apply_div<tu_uint8>(heapManager, lhs, rhs, result);
        case OperandType::UInt16:   return apply_div<tu_uint16>(heapManager, lhs, rhs, result);
        case OperandType::UInt32:   return apply_div<tu_uint32>(heapManager, lhs, rhs, result);
        case OperandType::UInt64:   return apply_div<tu_uint64>(heapManager, lhs, rhs, result);
        case OperandType::Int8:     return apply_div<tu_int8>(heapManager, lhs, rhs, result);
        case OperandType::Int16:    return apply_div<tu_int16>(heapManager, lhs, rhs, result);
        case OperandType::Int32:    return apply_div<tu_int32>(heapManager, lhs, rhs, result);
        case OperandType::Int64:    return apply_div<tu_int64>(heapManager, lhs, rhs, result);
        case OperandType::Float32:  return apply_div<float>(heapManager, lhs, rhs, result);
        case OperandType::Float64:  return apply_div<double>(heapManager, lhs, rhs, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::neg(HeapManager *heapManager, const Operand &element, Operand &result)
{
    switch (element.getType()) {
        case OperandType::UInt8:    return apply_neg<tu_uint8>(heapManager, element, result);
        case OperandType::UInt16:   return apply_neg<tu_uint16>(heapManager, element, result);
        case OperandType::UInt32:   return apply_neg<tu_uint32>(heapManager, element, result);
        case OperandType::UInt64:   return apply_neg<tu_uint64>(heapManager, element, result);
        case OperandType::Int8:     return apply_neg<tu_int8>(heapManager, element, result);
        case OperandType::Int16:    return apply_neg<tu_int16>(heapManager, element, result);
        case OperandType::Int32:    return apply_neg<tu_int32>(heapManager, element, result);
        case OperandType::Int64:    return apply_neg<tu_int64>(heapManager, element, result);
        case OperandType::Float32:  return apply_neg<float>(heapManager, element, result);
        case OperandType::Float64:  return apply_neg<double>(heapManager, element, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}