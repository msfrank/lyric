#ifndef LYRIC_RUNTIME_INTERNAL_I64_OPS_H
#define LYRIC_RUNTIME_INTERNAL_I64_OPS_H

#include <tempo_utils/status.h>

#include "../operand.h"

namespace lyric_runtime::internal {

    inline bool to_i64(const Operand &operand, tu_int64 &i64)
    {
        switch (operand.getType()) {
            case OperandType::Int64:
                return operand.getI64(i64);
            case OperandType::Int32:
                tu_int32 i32;
                if (!operand.getI32(i32))
                    return false;
                i64 = i32;
                return true;
            case OperandType::Int16:
                tu_int16 i16;
                if (!operand.getI16(i16))
                    return false;
                i64 = i16;
                return true;
            case OperandType::Int8:
                tu_int8 i8;
                if (!operand.getI8(i8))
                    return false;
                i64 = static_cast<unsigned char>(i8);
                return true;
            default:
                return false;
        }
    }

    tempo_utils::Status i64_add(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status i64_sub(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status i64_mul(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status i64_div(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status i64_cmp(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status i64_neg(const Operand &element, Operand &result);

    inline bool to_i32(const Operand &operand, tu_int32 &i32)
    {
        switch (operand.getType()) {
            case OperandType::Int32:
                return operand.getI32(i32);
            case OperandType::Int16:
                tu_int16 i16;
                if (!operand.getI16(i16))
                    return false;
                i32 = i16;
                return true;
            case OperandType::Int8:
                tu_int8 i8;
                if (!operand.getI8(i8))
                    return false;
                i32 = static_cast<unsigned char>(i8);
                return true;
            default:
                return false;
        }
    }

    tempo_utils::Status i32_add(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status i32_sub(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status i32_mul(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status i32_div(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status i32_cmp(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status i32_neg(const Operand &element, Operand &result);

    tempo_utils::Status i32_to_i16(const Operand &element, Operand &result);
    tempo_utils::Status i32_to_i8(const Operand &element, Operand &result);
}

#endif // LYRIC_RUNTIME_INTERNAL_I64_OPS_H
