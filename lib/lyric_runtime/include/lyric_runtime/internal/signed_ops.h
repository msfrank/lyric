#ifndef LYRIC_RUNTIME_INTERNAL_I64_OPS_H
#define LYRIC_RUNTIME_INTERNAL_I64_OPS_H

#include <lyric_runtime/data_cell.h>
#include <tempo_utils/status.h>

namespace lyric_runtime::internal {

    inline bool to_i64(const DataCell &cell, tu_int64 &i64)
    {
        switch (cell.type) {
            case DataCellType::Int8:
                i64 = static_cast<unsigned char>(cell.data.i8);
                return true;
            case DataCellType::Int16:
                i64 = cell.data.i16;
                return true;
            case DataCellType::Int32:
                i64 = cell.data.i32;
                return true;
            case DataCellType::Int64:
                i64 = cell.data.i64;
                return true;
            default:
                return false;
        }
    }

    tempo_utils::Status i64_add(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status i64_sub(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status i64_mul(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status i64_div(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status i64_cmp(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status i64_neg(const DataCell &element, DataCell &result);

    inline bool to_i32(const DataCell &cell, tu_int32 &i32)
    {
        switch (cell.type) {
            case DataCellType::Int8:
                i32 = static_cast<unsigned char>(cell.data.i8);
                return true;
            case DataCellType::Int16:
                i32 = cell.data.i16;
                return true;
            case DataCellType::Int32:
                i32 = cell.data.i32;
                return true;
            default:
                return false;
        }
    }

    tempo_utils::Status i32_add(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status i32_sub(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status i32_mul(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status i32_div(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status i32_cmp(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status i32_neg(const DataCell &element, DataCell &result);

    tempo_utils::Status i32_to_i16(const DataCell &element, DataCell &result);
    tempo_utils::Status i32_to_i8(const DataCell &element, DataCell &result);
}

#endif // LYRIC_RUNTIME_INTERNAL_I64_OPS_H
