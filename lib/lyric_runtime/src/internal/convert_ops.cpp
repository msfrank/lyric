
#include <cfenv>

#include <lyric_runtime/internal/convert_ops.h>
#include <lyric_runtime/internal/operand_helpers.h>

/**
 * Promote the operand `source`, which must contain a numeric value of type `SrcType`, to the
 * numeric type specified by `DstType`, and store the promoted value in `result`.
 *
 * @tparam SrcType The type of the value contained in `source`, which must be numeric and whose storage width
 *   must be smaller than the storage width of `DstType`.
 * @tparam DstType
 * @param source
 * @param result
 * @return
 */
template<class SrcType, class DstType>
tempo_utils::Status promote_value(lyric_runtime::HeapManager *heapManager, const lyric_runtime::Operand &source, lyric_runtime::Operand &result)
{
    static_assert (std::numeric_limits<SrcType>::digits < std::numeric_limits<DstType>::digits);

    SrcType s;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(source, s));
    lyric_runtime::value_to_operand(static_cast<DstType>(s), result, heapManager);
    return {};
}

template<class SrcType, class DstType>
tempo_utils::Status convert_integral_to_unsigned(lyric_runtime::HeapManager *heapManager, const lyric_runtime::Operand &source, lyric_runtime::Operand &result)
{
    static_assert(std::is_integral<SrcType>());
    static_assert(std::is_integral<DstType>() && std::is_unsigned<DstType>());

    SrcType s;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(source, s));
    lyric_runtime::value_to_operand(static_cast<DstType>(s), result, heapManager);
    return {};
}

template<class SrcType, class DstType>
tempo_utils::Status convert_integral_to_signed(lyric_runtime::HeapManager *heapManager, const lyric_runtime::Operand &source, lyric_runtime::Operand &result)
{
    static_assert(std::is_integral<SrcType>());
    static_assert(std::is_integral<DstType>() && std::is_signed<DstType>());

    SrcType s;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(source, s));
    lyric_runtime::value_to_operand(static_cast<DstType>(s), result, heapManager);
    return {};
}

template<class SrcType, class DstType>
tempo_utils::Status convert_floating_to_integral(lyric_runtime::HeapManager *heapManager, const lyric_runtime::Operand &source, lyric_runtime::Operand &result)
{
    static_assert(std::is_floating_point<SrcType>());
    static_assert(std::is_integral<DstType>());

    SrcType s;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(source, s));

    //
    if (std::isnormal(s)) [[likely]] {
        if (s >= static_cast<SrcType>(std::numeric_limits<DstType>::max())) [[unlikely]] {
            lyric_runtime::value_to_operand(std::numeric_limits<DstType>::max(), result, heapManager);
        } else if (s <= static_cast<SrcType>(std::numeric_limits<DstType>::min())) [[unlikely]] {
            lyric_runtime::value_to_operand(std::numeric_limits<DstType>::min(), result, heapManager);
        } else [[likely]] {
            lyric_runtime::value_to_operand(static_cast<DstType>(s), result, heapManager);
        }
        return {};
    }

    switch (std::fpclassify(s)) {
        // NaN and +/-0.0 is converted to 0
        case FP_NAN:
        case FP_ZERO:
            lyric_runtime::value_to_operand(static_cast<DstType>(0), result, heapManager);
            return {};
        // +inf saturates to the maximum allowed value of DstType.
        // -inf saturates to the minimum allowed value of DstType.
        case FP_INFINITE: {
            if (std::signbit(s)) {
                lyric_runtime::value_to_operand(std::numeric_limits<DstType>::min(), result, heapManager);
            } else {
                lyric_runtime::value_to_operand(std::numeric_limits<DstType>::max(), result, heapManager);
            }
            return {};
        }
        case FP_SUBNORMAL:
        default:
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kInvalidDataStackV1, "invalid source value");
    }
}

template<class SrcType, class DstType>
tempo_utils::Status convert_integral_to_floating(lyric_runtime::HeapManager *heapManager, const lyric_runtime::Operand &source, lyric_runtime::Operand &result)
{
    static_assert(std::is_integral<SrcType>());
    static_assert(std::is_floating_point<DstType>());

    SrcType s;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(source, s));

    // if source is larger than the maximum representable DstType value then return +inf
    if (s >= 0 && static_cast<tu_uint64>(s) > static_cast<tu_uint64>(std::numeric_limits<DstType>::max())) {
        lyric_runtime::value_to_operand(std::numeric_limits<DstType>::infinity(), result, heapManager);
        return {};
    }

    // if source is smaller than the minimum representable DstType value then return -inf
    if (s < 0 && static_cast<tu_int64>(s) < static_cast<tu_int64>(std::numeric_limits<DstType>::min())) {
        lyric_runtime::value_to_operand(-std::numeric_limits<DstType>::infinity(), result, heapManager);
        return {};
    }

    // otherwise set rounding mode to nearest and perform conversion
    const int prevmode = std::fegetround();
    std::fesetround(FE_TONEAREST);
    lyric_runtime::value_to_operand(static_cast<DstType>(std::nearbyint(s)), result, heapManager);
    std::fesetround(prevmode);
    return {};
}

tempo_utils::Status convert_double_to_float(lyric_runtime::HeapManager *heapManager, const lyric_runtime::Operand &source, lyric_runtime::Operand &result)
{
    double s;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(source, s));

    // if source is larger than the maximum representable DstType value then return +inf
    if (s >= 0 && static_cast<double>(s) > static_cast<double>(std::numeric_limits<float>::max())) {
        lyric_runtime::value_to_operand(std::numeric_limits<float>::infinity(), result, heapManager);
        return {};
    }

    // if source is smaller than the minimum representable DstType value then return -inf
    if (s < 0 && static_cast<double>(s) < static_cast<double>(std::numeric_limits<float>::min())) {
        lyric_runtime::value_to_operand(-std::numeric_limits<float>::infinity(), result, heapManager);
        return {};
    }

    lyric_runtime::value_to_operand(static_cast<float>(s), result, heapManager);
    return {};
}

tempo_utils::Status
lyric_runtime::internal::convert_to_I8(HeapManager *heapManager, const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::Int8:
            result = source;
            return {};
        case OperandType::Int16:
            return convert_integral_to_signed<tu_int16,tu_int8>(heapManager, source, result);
        case OperandType::Int32:
            return convert_integral_to_signed<tu_int32,tu_int8>(heapManager, source, result);
        case OperandType::Int64:
            return convert_integral_to_signed<tu_int64,tu_int8>(heapManager, source, result);
        case OperandType::UInt8:
            return convert_integral_to_signed<tu_uint8,tu_int8>(heapManager, source, result);
        case OperandType::UInt16:
            return convert_integral_to_signed<tu_uint16,tu_int8>(heapManager, source, result);
        case OperandType::UInt32:
            return convert_integral_to_signed<tu_uint32,tu_int8>(heapManager, source, result);
        case OperandType::UInt64:
            return convert_integral_to_signed<tu_uint64,tu_int8>(heapManager, source, result);
        case OperandType::Float32:
            return convert_floating_to_integral<float,tu_int8>(heapManager, source, result);
        case OperandType::Float64:
            return convert_floating_to_integral<double,tu_int8>(heapManager, source, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}

tempo_utils::Status
lyric_runtime::internal::convert_to_I16(HeapManager *heapManager, const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::Int8:
            return promote_value<tu_int8,tu_int16>(heapManager, source, result);
        case OperandType::Int16:
            result = source;
            return {};
        case OperandType::Int32:
            return convert_integral_to_signed<tu_int32,tu_int16>(heapManager, source, result);
        case OperandType::Int64:
            return convert_integral_to_signed<tu_int64,tu_int16>(heapManager, source, result);
        case OperandType::UInt8:
            return convert_integral_to_signed<tu_uint8,tu_int16>(heapManager, source, result);
        case OperandType::UInt16:
            return convert_integral_to_signed<tu_uint16,tu_int16>(heapManager, source, result);
        case OperandType::UInt32:
            return convert_integral_to_signed<tu_uint32,tu_int16>(heapManager, source, result);
        case OperandType::UInt64:
            return convert_integral_to_signed<tu_uint64,tu_int16>(heapManager, source, result);
        case OperandType::Float32:
            return convert_floating_to_integral<float,tu_int16>(heapManager, source, result);
        case OperandType::Float64:
            return convert_floating_to_integral<double,tu_int16>(heapManager, source, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}

tempo_utils::Status
lyric_runtime::internal::convert_to_I32(HeapManager *heapManager, const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::Int8:
            return promote_value<tu_int8,tu_int32>(heapManager, source, result);
        case OperandType::Int16:
            return promote_value<tu_int16,tu_int32>(heapManager, source, result);
        case OperandType::Int32:
            result = source;
            return {};
        case OperandType::Int64:
            return convert_integral_to_signed<tu_int64,tu_int32>(heapManager, source, result);
        case OperandType::UInt8:
            return convert_integral_to_signed<tu_uint8,tu_int32>(heapManager, source, result);
        case OperandType::UInt16:
            return convert_integral_to_signed<tu_uint16,tu_int32>(heapManager, source, result);
        case OperandType::UInt32:
            return convert_integral_to_signed<tu_uint32,tu_int32>(heapManager, source, result);
        case OperandType::UInt64:
            return convert_integral_to_signed<tu_uint64,tu_int32>(heapManager, source, result);
        case OperandType::Float32:
            return convert_floating_to_integral<float,tu_int32>(heapManager, source, result);
        case OperandType::Float64:
            return convert_floating_to_integral<double,tu_int32>(heapManager, source, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}

tempo_utils::Status
lyric_runtime::internal::convert_to_I64(HeapManager *heapManager, const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::Int8:
            return promote_value<tu_int8,tu_int64>(heapManager, source, result);
        case OperandType::Int16:
            return promote_value<tu_int16,tu_int64>(heapManager, source, result);
        case OperandType::Int32:
            return promote_value<tu_int32,tu_int64>(heapManager, source, result);
        case OperandType::Int64:
            result = source;
            return {};
        case OperandType::UInt8:
            return convert_integral_to_signed<tu_uint8,tu_int64>(heapManager, source, result);
        case OperandType::UInt16:
            return convert_integral_to_signed<tu_uint16,tu_int64>(heapManager, source, result);
        case OperandType::UInt32:
            return convert_integral_to_signed<tu_uint32,tu_int64>(heapManager, source, result);
        case OperandType::UInt64:
            return convert_integral_to_signed<tu_uint64,tu_int64>(heapManager, source, result);
        case OperandType::Float32:
            return convert_floating_to_integral<float,tu_int64>(heapManager, source, result);
        case OperandType::Float64:
            return convert_floating_to_integral<double,tu_int64>(heapManager, source, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}

tempo_utils::Status
lyric_runtime::internal::convert_to_U8(HeapManager *heapManager, const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::Int8:
            return convert_integral_to_unsigned<tu_int8,tu_uint8>(heapManager, source, result);
        case OperandType::Int16:
            return convert_integral_to_unsigned<tu_int16,tu_uint8>(heapManager, source, result);
        case OperandType::Int32:
            return convert_integral_to_unsigned<tu_int32,tu_uint8>(heapManager, source, result);
        case OperandType::Int64:
            return convert_integral_to_unsigned<tu_int64,tu_uint8>(heapManager, source, result);
        case OperandType::UInt8:
            result = source;
            return {};
        case OperandType::UInt16:
            return convert_integral_to_unsigned<tu_uint16,tu_uint8>(heapManager, source, result);
        case OperandType::UInt32:
            return convert_integral_to_unsigned<tu_uint32,tu_uint8>(heapManager, source, result);
        case OperandType::UInt64:
            return convert_integral_to_unsigned<tu_uint64,tu_uint8>(heapManager, source, result);
        case OperandType::Float32:
            return convert_floating_to_integral<float,tu_uint8>(heapManager, source, result);
        case OperandType::Float64:
            return convert_floating_to_integral<double,tu_uint8>(heapManager, source, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}

tempo_utils::Status
lyric_runtime::internal::convert_to_U16(HeapManager *heapManager, const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::Int8:
            return convert_integral_to_unsigned<tu_int8,tu_uint16>(heapManager, source, result);
        case OperandType::Int16:
            return convert_integral_to_unsigned<tu_int16,tu_uint16>(heapManager, source, result);
        case OperandType::Int32:
            return convert_integral_to_unsigned<tu_int32,tu_uint16>(heapManager, source, result);
        case OperandType::Int64:
            return convert_integral_to_unsigned<tu_int64,tu_uint16>(heapManager, source, result);
        case OperandType::UInt8:
            return promote_value<tu_uint8,tu_uint16>(heapManager, source, result);
        case OperandType::UInt16:
            result = source;
            return {};
        case OperandType::UInt32:
            return convert_integral_to_unsigned<tu_uint32,tu_uint16>(heapManager, source, result);
        case OperandType::UInt64:
            return convert_integral_to_unsigned<tu_uint64,tu_uint16>(heapManager, source, result);
        case OperandType::Float32:
            return convert_floating_to_integral<float,tu_uint16>(heapManager, source, result);
        case OperandType::Float64:
            return convert_floating_to_integral<double,tu_uint16>(heapManager, source, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}

tempo_utils::Status
lyric_runtime::internal::convert_to_U32(HeapManager *heapManager, const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::Int8:
            return convert_integral_to_unsigned<tu_int8,tu_uint32>(heapManager, source, result);
        case OperandType::Int16:
            return convert_integral_to_unsigned<tu_int16,tu_uint32>(heapManager, source, result);
        case OperandType::Int32:
            return convert_integral_to_unsigned<tu_int32,tu_uint32>(heapManager, source, result);
        case OperandType::Int64:
            return convert_integral_to_unsigned<tu_int64,tu_uint32>(heapManager, source, result);
        case OperandType::UInt8:
            return promote_value<tu_uint8,tu_uint32>(heapManager, source, result);
        case OperandType::UInt16:
            return promote_value<tu_uint16,tu_uint32>(heapManager, source, result);
        case OperandType::UInt32:
            result = source;
            return {};
        case OperandType::UInt64:
            return convert_integral_to_unsigned<tu_uint64,tu_uint32>(heapManager, source, result);
        case OperandType::Float32:
            return convert_floating_to_integral<float,tu_uint32>(heapManager, source, result);
        case OperandType::Float64:
            return convert_floating_to_integral<double,tu_uint32>(heapManager, source, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}

tempo_utils::Status
lyric_runtime::internal::convert_to_U64(HeapManager *heapManager, const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::Int8:
            return convert_integral_to_unsigned<tu_int8,tu_uint64>(heapManager, source, result);
        case OperandType::Int16:
            return convert_integral_to_unsigned<tu_int16,tu_uint64>(heapManager, source, result);
        case OperandType::Int32:
            return convert_integral_to_unsigned<tu_int32,tu_uint64>(heapManager, source, result);
        case OperandType::Int64:
            return convert_integral_to_unsigned<tu_int64,tu_uint64>(heapManager, source, result);
        case OperandType::UInt8:
            return promote_value<tu_uint8,tu_uint64>(heapManager, source, result);
        case OperandType::UInt16:
            return promote_value<tu_uint16,tu_uint64>(heapManager, source, result);
        case OperandType::UInt32:
            return promote_value<tu_uint32,tu_uint64>(heapManager, source, result);
        case OperandType::UInt64:
            result = source;
            return {};
        case OperandType::Float32:
            return convert_floating_to_integral<float,tu_uint64>(heapManager, source, result);
        case OperandType::Float64:
            return convert_floating_to_integral<double,tu_uint64>(heapManager, source, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}

tempo_utils::Status
lyric_runtime::internal::convert_to_F32(HeapManager *heapManager, const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::Int8:
            return convert_integral_to_floating<tu_int8,double>(heapManager, source, result);
        case OperandType::Int16:
            return convert_integral_to_floating<tu_int16,double>(heapManager, source, result);
        case OperandType::Int32:
            return convert_integral_to_floating<tu_int32,double>(heapManager, source, result);
        case OperandType::Int64:
            return convert_integral_to_floating<tu_int64,double>(heapManager, source, result);
        case OperandType::UInt8:
            return convert_integral_to_floating<tu_uint8,double>(heapManager, source, result);
        case OperandType::UInt16:
            return convert_integral_to_floating<tu_uint16,double>(heapManager, source, result);
        case OperandType::UInt32:
            return convert_integral_to_floating<tu_uint32,double>(heapManager, source, result);
        case OperandType::UInt64:
            return convert_integral_to_floating<tu_uint64,double>(heapManager, source, result);
        case OperandType::Float32:
            result = source;
            return {};
        case OperandType::Float64:
            return convert_double_to_float(heapManager, source, result);
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}

tempo_utils::Status
lyric_runtime::internal::convert_to_F64(HeapManager *heapManager, const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::Int8:
            return convert_integral_to_floating<tu_int8,double>(heapManager, source, result);
        case OperandType::Int16:
            return convert_integral_to_floating<tu_int16,double>(heapManager, source, result);
        case OperandType::Int32:
            return convert_integral_to_floating<tu_int32,double>(heapManager, source, result);
        case OperandType::Int64:
            return convert_integral_to_floating<tu_int64,double>(heapManager, source, result);
        case OperandType::UInt8:
            return convert_integral_to_floating<tu_uint8,double>(heapManager, source, result);
        case OperandType::UInt16:
            return convert_integral_to_floating<tu_uint16,double>(heapManager, source, result);
        case OperandType::UInt32:
            return convert_integral_to_floating<tu_uint32,double>(heapManager, source, result);
        case OperandType::UInt64:
            return convert_integral_to_floating<tu_uint64,double>(heapManager, source, result);
        case OperandType::Float32:
            return promote_value<float,double>(heapManager, source, result);
        case OperandType::Float64:
            result = source;
            return {};
        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}