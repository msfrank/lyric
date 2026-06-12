#ifndef LYRIC_RUNTIME_INTERNAL_NUMERIC_OPS_H
#define LYRIC_RUNTIME_INTERNAL_NUMERIC_OPS_H

#include <tempo_utils/status.h>

#include "../operand.h"

namespace lyric_runtime::internal {

    tempo_utils::Status add(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status sub(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status mul(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status div(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status neg(const Operand &element, Operand &result);
}

#endif // LYRIC_RUNTIME_INTERNAL_NUMERIC_OPS_H
