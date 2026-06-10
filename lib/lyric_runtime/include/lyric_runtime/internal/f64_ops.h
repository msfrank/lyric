#ifndef LYRIC_RUNTIME_INTERNAL_F64_OPS_H
#define LYRIC_RUNTIME_INTERNAL_F64_OPS_H

#include <tempo_utils/status.h>

#include "../operand.h"

namespace lyric_runtime::internal {

    tempo_utils::Status f64_add(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status f64_sub(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status f64_mul(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status f64_div(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status f64_cmp(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status f64_neg(const Operand &v1, Operand &result);
}

#endif // LYRIC_RUNTIME_INTERNAL_F64_OPS_H
