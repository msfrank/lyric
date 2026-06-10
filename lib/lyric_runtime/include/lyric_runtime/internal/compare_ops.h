#ifndef LYRIC_RUNTIME_INTERNAL_COMPARE_OPS_H
#define LYRIC_RUNTIME_INTERNAL_COMPARE_OPS_H

#include <tempo_utils/status.h>

#include "../operand.h"

namespace lyric_runtime::internal {

    tempo_utils::Status is_zero(const Operand &element, bool &result);
    tempo_utils::Status is_not_zero(const Operand &element, bool &result);
    tempo_utils::Status is_greater_than(const Operand &element, bool &result);
    tempo_utils::Status is_greater_or_equal(const Operand &element, bool &result);
    tempo_utils::Status is_less_than(const Operand &element, bool &result);
    tempo_utils::Status is_less_or_equal(const Operand &element, bool &result);
}

#endif // LYRIC_RUNTIME_INTERNAL_COMPARE_OPS_H
