#ifndef LYRIC_RUNTIME_INTERNAL_NUMERIC_OPS_H
#define LYRIC_RUNTIME_INTERNAL_NUMERIC_OPS_H

#include <tempo_utils/status.h>

#include "../heap_manager.h"
#include "../operand.h"

namespace lyric_runtime::internal {

    tempo_utils::Status add(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status sub(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status mul(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status div(HeapManager *heapManager, const Operand &lhs, const Operand &rhs, Operand &result);
    tempo_utils::Status neg(HeapManager *heapManager, const Operand &element, Operand &result);
}

#endif // LYRIC_RUNTIME_INTERNAL_NUMERIC_OPS_H
