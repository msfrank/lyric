#ifndef LYRIC_RUNTIME_INTERNAL_BITWISE_OPS_H
#define LYRIC_RUNTIME_INTERNAL_BITWISE_OPS_H

#include <tempo_utils/status.h>

#include "../operand.h"

namespace lyric_runtime::internal {

    tempo_utils::Status bitwise_and(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status bitwise_or(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status bitwise_xor(const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status bitwise_not(const Operand &element, Operand &result);
    tempo_utils::Status bitwise_shl(const Operand &element, const Operand &count, Operand &result);
    tempo_utils::Status bitwise_shr(const Operand &element, const Operand &count, Operand &result);
}

#endif // LYRIC_RUNTIME_INTERNAL_BITWISE_OPS_H
