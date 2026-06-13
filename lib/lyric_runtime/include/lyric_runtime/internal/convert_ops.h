#ifndef LYRIC_RUNTIME_INTERNAL_CONVERT_OPS_H
#define LYRIC_RUNTIME_INTERNAL_CONVERT_OPS_H

#include <tempo_utils/status.h>

#include "../operand.h"

namespace lyric_runtime::internal {

    tempo_utils::Status convert_to_I8(const Operand &source, Operand &result);
    tempo_utils::Status convert_to_I16(const Operand &source, Operand &result);
    tempo_utils::Status convert_to_I32(const Operand &source, Operand &result);
    tempo_utils::Status convert_to_I64(const Operand &source, Operand &result);

    tempo_utils::Status convert_to_U8(const Operand &source, Operand &result);
    tempo_utils::Status convert_to_U16(const Operand &source, Operand &result);
    tempo_utils::Status convert_to_U32(const Operand &source, Operand &result);
    tempo_utils::Status convert_to_U64(const Operand &source, Operand &result);

    tempo_utils::Status convert_to_F32(const Operand &source, Operand &result);
    tempo_utils::Status convert_to_F64(const Operand &source, Operand &result);
}

#endif // LYRIC_RUNTIME_INTERNAL_CONVERT_OPS_H
