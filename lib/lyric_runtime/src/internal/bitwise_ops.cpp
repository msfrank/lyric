
#include <lyric_runtime/internal/bitwise_ops.h>
#include <lyric_runtime/internal/operand_helpers.h>
#include <lyric_runtime/interpreter_result.h>

template<class ValueType>
tempo_utils::Status apply_bitwise_and(
    lyric_runtime::HeapManager *heapManager,
    const lyric_runtime::Operand &lhs,
    const lyric_runtime::Operand &rhs,
    lyric_runtime::Operand &result)
{
    ValueType l, r;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_binary_operands(lhs, rhs, l, r));
    value_to_operand(l & r, result, heapManager);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_bitwise_or(
    lyric_runtime::HeapManager *heapManager,
    const lyric_runtime::Operand &lhs,
    const lyric_runtime::Operand &rhs,
    lyric_runtime::Operand &result)
{
    ValueType l, r;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_binary_operands(lhs, rhs, l, r));
    value_to_operand(l | r, result, heapManager);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_bitwise_xor(
    lyric_runtime::HeapManager *heapManager,
    const lyric_runtime::Operand &lhs,
    const lyric_runtime::Operand &rhs,
    lyric_runtime::Operand &result)
{
    ValueType l, r;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_binary_operands(lhs, rhs, l, r));
    value_to_operand(l ^ r, result, heapManager);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_bitwise_not(
    lyric_runtime::HeapManager *heapManager,
    const lyric_runtime::Operand &element,
    lyric_runtime::Operand &result)
{
    ValueType e;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    value_to_operand(~e, result, heapManager);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_bitwise_shl(
    lyric_runtime::HeapManager *heapManager,
    const lyric_runtime::Operand &element,
    const lyric_runtime::Operand &count,
    lyric_runtime::Operand &result)
{
    ValueType e;
    tu_uint8 c;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(count, c));
    if (std::numeric_limits<ValueType>::digits <= c)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kInvalidDataStackV2,
            "invalid shift count");
    value_to_operand(e << c, result, heapManager);
    return {};
}

template<class ValueType>
tempo_utils::Status apply_bitwise_shr(
    lyric_runtime::HeapManager *heapManager,
    const lyric_runtime::Operand &element,
    const lyric_runtime::Operand &count,
    lyric_runtime::Operand &result)
{
    ValueType e;
    tu_uint8 c;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(element, e));
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(count, c));
    if (std::numeric_limits<ValueType>::digits <= c)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kInvalidDataStackV2,
            "invalid shift count");
    value_to_operand(e >> c, result, heapManager);
    return {};
}

tempo_utils::Status
lyric_runtime::internal::bitwise_and(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8: {
            return apply_bitwise_and<tu_uint8>(heapManager, lhs, rhs, result);
        }
        case OperandType::UInt16: {
            return apply_bitwise_and<tu_uint16>(heapManager, lhs, rhs, result);
        }
        case OperandType::UInt32: {
            return apply_bitwise_and<tu_uint32>(heapManager, lhs, rhs, result);
        }
        case OperandType::UInt64: {
            return apply_bitwise_and<tu_uint64>(heapManager, lhs, rhs, result);
        }
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::bitwise_or(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8: {
            return apply_bitwise_or<tu_uint8>(heapManager, lhs, rhs, result);
        }
        case OperandType::UInt16: {
            return apply_bitwise_or<tu_uint16>(heapManager, lhs, rhs, result);
        }
        case OperandType::UInt32: {
            return apply_bitwise_or<tu_uint32>(heapManager, lhs, rhs, result);
        }
        case OperandType::UInt64: {
            return apply_bitwise_or<tu_uint64>(heapManager, lhs, rhs, result);
        }
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::bitwise_xor(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result)
{
    switch (lhs.getType()) {
        case OperandType::UInt8: {
            return apply_bitwise_xor<tu_uint8>(heapManager, lhs, rhs, result);
        }
        case OperandType::UInt16: {
            return apply_bitwise_xor<tu_uint16>(heapManager, lhs, rhs, result);
        }
        case OperandType::UInt32: {
            return apply_bitwise_xor<tu_uint32>(heapManager, lhs, rhs, result);
        }
        case OperandType::UInt64: {
            return apply_bitwise_xor<tu_uint64>(heapManager, lhs, rhs, result);
        }
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::bitwise_not(HeapManager *heapManager, const Operand &element, Operand &result)
{
    switch (element.getType()) {
        case OperandType::UInt8: {
            return apply_bitwise_not<tu_uint8>(heapManager, element, result);
        }
        case OperandType::UInt16: {
            return apply_bitwise_not<tu_uint16>(heapManager, element, result);
        }
        case OperandType::UInt32: {
            return apply_bitwise_not<tu_uint32>(heapManager, element, result);
        }
        case OperandType::UInt64: {
            return apply_bitwise_not<tu_uint64>(heapManager, element, result);
        }
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs value");
    }
}

tempo_utils::Status
lyric_runtime::internal::bitwise_shl(HeapManager *heapManager, const Operand &element, const Operand &count, Operand &result)
{
    switch (element.getType()) {
        case OperandType::UInt8: {
            return apply_bitwise_shl<tu_uint8>(heapManager, element, count, result);
        }
        case OperandType::UInt16: {
            return apply_bitwise_shl<tu_uint16>(heapManager, element, count, result);
        }
        case OperandType::UInt32: {
            return apply_bitwise_shl<tu_uint32>(heapManager, element, count, result);
        }
        case OperandType::UInt64: {
            return apply_bitwise_shl<tu_uint64>(heapManager, element, count, result);
        }
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid shift value");
    }
}

tempo_utils::Status
lyric_runtime::internal::bitwise_shr(HeapManager *heapManager, const Operand &element, const Operand &count, Operand &result)
{
    switch (element.getType()) {
        case OperandType::UInt8: {
            return apply_bitwise_shr<tu_uint8>(heapManager, element, count, result);
        }
        case OperandType::UInt16: {
            return apply_bitwise_shr<tu_uint16>(heapManager, element, count, result);
        }
        case OperandType::UInt32: {
            return apply_bitwise_shr<tu_uint32>(heapManager, element, count, result);
        }
        case OperandType::UInt64: {
            return apply_bitwise_shr<tu_uint64>(heapManager, element, count, result);
        }
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid shift value");
    }
}
