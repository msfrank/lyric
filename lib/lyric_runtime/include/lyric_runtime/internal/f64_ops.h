#ifndef LYRIC_RUNTIME_INTERNAL_F64_OPS_H
#define LYRIC_RUNTIME_INTERNAL_F64_OPS_H

#include <lyric_runtime/data_cell.h>
#include <tempo_utils/status.h>

namespace lyric_runtime::internal {

    tempo_utils::Status f64_add(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status f64_sub(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status f64_mul(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status f64_div(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status f64_cmp(const DataCell &lhs, const DataCell &rhs, DataCell &result);
    tempo_utils::Status f64_neg(const DataCell &v1, DataCell &result);
}

#endif // LYRIC_RUNTIME_INTERNAL_F64_OPS_H
