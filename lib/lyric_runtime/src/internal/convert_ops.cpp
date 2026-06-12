
#include <lyric_runtime/internal/convert_ops.h>
#include <lyric_runtime/internal/operand_helpers.h>

template<class SrcType, class DstType>
tempo_utils::Status promote_value(const lyric_runtime::Operand &source, lyric_runtime::Operand &result)
{
    SrcType s;
    TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(source, s));
    lyric_runtime::value_to_operand(static_cast<DstType>(s), result);
    return {};
}

// template<class SrcType, class DstType>
// tempo_utils::Status convert_integral(const lyric_runtime::Operand &source, lyric_runtime::Operand &result)
// {
//     SrcType s;
//     TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(source, s));
//     lyric_runtime::value_to_operand(static_cast<DstType>(s), result);
//     return {};
// }
//
// template<class SrcType, class DstType>
// tempo_utils::Status convert_rational_to_integral(const lyric_runtime::Operand &source, lyric_runtime::Operand &result)
// {
//     SrcType s;
//     TU_RETURN_IF_NOT_OK (lyric_runtime::internal::get_unary_operand(source, s));
//     tu_int64 i64 = std::floor(s);
//     lyric_runtime::value_to_operand(static_cast<DstType>(s), result);
//     return {};
// }

tempo_utils::Status
lyric_runtime::internal::convert_to_U64(const Operand &source, Operand &result)
{
    switch (source.getType()) {
        case OperandType::UInt8:
            return promote_value<tu_uint8,tu_uint64>(source, result);
        case OperandType::UInt16:
            return promote_value<tu_uint16,tu_uint64>(source, result);
        case OperandType::UInt32:
            return promote_value<tu_uint32,tu_uint64>(source, result);
        case OperandType::UInt64:
            result = source;
            return {};

        default:
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid source value");
    }
}