#ifndef LYRIC_RUNTIME_INTERNAL_BITWISE_OPS_H
#define LYRIC_RUNTIME_INTERNAL_BITWISE_OPS_H

#include <tempo_utils/status.h>

#include "../heap_manager.h"
#include "../operand.h"

namespace lyric_runtime::internal {

    tempo_utils::Status bitwise_and(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status bitwise_or(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status bitwise_xor(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status bitwise_not(HeapManager *heapManager, const Operand &element, Operand &result);
    tempo_utils::Status bitwise_shl(HeapManager *heapManager, const Operand &element, const Operand &count, Operand &result);
    tempo_utils::Status bitwise_shr(HeapManager *heapManager, const Operand &element, const Operand &count, Operand &result);
}

#endif // LYRIC_RUNTIME_INTERNAL_BITWISE_OPS_H
