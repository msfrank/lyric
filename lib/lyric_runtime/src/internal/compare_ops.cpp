
#include <lyric_runtime/internal/compare_ops.h>
#include <lyric_runtime/internal/operand_helpers.h>
#include <lyric_runtime/interpreter_result.h>

template<class ValueType>
tempo_utils::Status apply_is_zero(
    const lyric_runtime::Operand &element,
    bool &result)
{
    ValueType e;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    result = e == 0;
    return {};
}

template<class ValueType>
tempo_utils::Status apply_is_not_zero(
    const lyric_runtime::Operand &element,
    bool &result)
{
    ValueType e;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    result = e != 0;
    return {};
}

template<class ValueType>
tempo_utils::Status apply_is_greater_than(
    const lyric_runtime::Operand &element,
    bool &result)
{
    ValueType e;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    result = e > 0;
    return {};
}

template<class ValueType>
tempo_utils::Status apply_is_greater_or_equal(
    const lyric_runtime::Operand &element,
    bool &result)
{
    ValueType e;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    result = e >= 0;
    return {};
}

template<class ValueType>
tempo_utils::Status apply_is_less_than(
    const lyric_runtime::Operand &element,
    bool &result)
{
    ValueType e;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    result = e < 0;
    return {};
}

template<class ValueType>
tempo_utils::Status apply_is_less_or_equal(
    const lyric_runtime::Operand &element,
    bool &result)
{
    ValueType e;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    result = e <= 0;
    return {};
}

tempo_utils::Status
lyric_runtime::internal::is_zero(const Operand &element, bool &result)
{
    switch (element.getType()) {
        case OperandType::UInt8:   return apply_is_zero<tu_uint8>(element, result);
        case OperandType::UInt16:  return apply_is_zero<tu_uint16>(element, result);
        case OperandType::UInt32:  return apply_is_zero<tu_uint32>(element, result);
        case OperandType::UInt64:  return apply_is_zero<tu_uint64>(element, result);
        case OperandType::Int8:    return apply_is_zero<tu_int8>(element, result);
        case OperandType::Int16:   return apply_is_zero<tu_int16>(element, result);
        case OperandType::Int32:   return apply_is_zero<tu_int32>(element, result);
        case OperandType::Int64:   return apply_is_zero<tu_int64>(element, result);
        case OperandType::Float32: return apply_is_zero<float>(element, result);
        case OperandType::Float64: return apply_is_zero<double>(element, result);

        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::is_not_zero(const Operand &element, bool &result)
{
    switch (element.getType()) {
        case OperandType::UInt8:   return apply_is_not_zero<tu_uint8>(element, result);
        case OperandType::UInt16:  return apply_is_not_zero<tu_uint16>(element, result);
        case OperandType::UInt32:  return apply_is_not_zero<tu_uint32>(element, result);
        case OperandType::UInt64:  return apply_is_not_zero<tu_uint64>(element, result);
        case OperandType::Int8:    return apply_is_not_zero<tu_int8>(element, result);
        case OperandType::Int16:   return apply_is_not_zero<tu_int16>(element, result);
        case OperandType::Int32:   return apply_is_not_zero<tu_int32>(element, result);
        case OperandType::Int64:   return apply_is_not_zero<tu_int64>(element, result);
        case OperandType::Float32: return apply_is_not_zero<float>(element, result);
        case OperandType::Float64: return apply_is_not_zero<double>(element, result);

        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status lyric_runtime::internal::is_greater_than(const Operand &element, bool &result)
{
    switch (element.getType()) {
        case OperandType::UInt8:   return apply_is_greater_than<tu_uint8>(element, result);
        case OperandType::UInt16:  return apply_is_greater_than<tu_uint16>(element, result);
        case OperandType::UInt32:  return apply_is_greater_than<tu_uint32>(element, result);
        case OperandType::UInt64:  return apply_is_greater_than<tu_uint64>(element, result);
        case OperandType::Int8:    return apply_is_greater_than<tu_int8>(element, result);
        case OperandType::Int16:   return apply_is_greater_than<tu_int16>(element, result);
        case OperandType::Int32:   return apply_is_greater_than<tu_int32>(element, result);
        case OperandType::Int64:   return apply_is_greater_than<tu_int64>(element, result);
        case OperandType::Float32: return apply_is_greater_than<float>(element, result);
        case OperandType::Float64: return apply_is_greater_than<double>(element, result);

        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status lyric_runtime::internal::is_greater_or_equal(const Operand &element, bool &result)
{
    switch (element.getType()) {
        case OperandType::UInt8:   return apply_is_greater_or_equal<tu_uint8>(element, result);
        case OperandType::UInt16:  return apply_is_greater_or_equal<tu_uint16>(element, result);
        case OperandType::UInt32:  return apply_is_greater_or_equal<tu_uint32>(element, result);
        case OperandType::UInt64:  return apply_is_greater_or_equal<tu_uint64>(element, result);
        case OperandType::Int8:    return apply_is_greater_or_equal<tu_int8>(element, result);
        case OperandType::Int16:   return apply_is_greater_or_equal<tu_int16>(element, result);
        case OperandType::Int32:   return apply_is_greater_or_equal<tu_int32>(element, result);
        case OperandType::Int64:   return apply_is_greater_or_equal<tu_int64>(element, result);
        case OperandType::Float32: return apply_is_greater_or_equal<float>(element, result);
        case OperandType::Float64: return apply_is_greater_or_equal<double>(element, result);

        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status lyric_runtime::internal::is_less_than(const Operand &element, bool &result)
{
    switch (element.getType()) {
        case OperandType::UInt8:   return apply_is_less_than<tu_uint8>(element, result);
        case OperandType::UInt16:  return apply_is_less_than<tu_uint16>(element, result);
        case OperandType::UInt32:  return apply_is_less_than<tu_uint32>(element, result);
        case OperandType::UInt64:  return apply_is_less_than<tu_uint64>(element, result);
        case OperandType::Int8:    return apply_is_less_than<tu_int8>(element, result);
        case OperandType::Int16:   return apply_is_less_than<tu_int16>(element, result);
        case OperandType::Int32:   return apply_is_less_than<tu_int32>(element, result);
        case OperandType::Int64:   return apply_is_less_than<tu_int64>(element, result);
        case OperandType::Float32: return apply_is_less_than<float>(element, result);
        case OperandType::Float64: return apply_is_less_than<double>(element, result);

        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status lyric_runtime::internal::is_less_or_equal(const Operand &element, bool &result)
{
    switch (element.getType()) {
        case OperandType::UInt8:   return apply_is_less_or_equal<tu_uint8>(element, result);
        case OperandType::UInt16:  return apply_is_less_or_equal<tu_uint16>(element, result);
        case OperandType::UInt32:  return apply_is_less_or_equal<tu_uint32>(element, result);
        case OperandType::UInt64:  return apply_is_less_or_equal<tu_uint64>(element, result);
        case OperandType::Int8:    return apply_is_less_or_equal<tu_int8>(element, result);
        case OperandType::Int16:   return apply_is_less_or_equal<tu_int16>(element, result);
        case OperandType::Int32:   return apply_is_less_or_equal<tu_int32>(element, result);
        case OperandType::Int64:   return apply_is_less_or_equal<tu_int64>(element, result);
        case OperandType::Float32: return apply_is_less_or_equal<float>(element, result);
        case OperandType::Float64: return apply_is_less_or_equal<double>(element, result);

        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}