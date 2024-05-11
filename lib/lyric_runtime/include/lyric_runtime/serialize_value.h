#ifndef LYRIC_RUNTIME_SERIALIZE_VALUE_H
#define LYRIC_RUNTIME_SERIALIZE_VALUE_H

#include <lyric_serde/patchset_state.h>
#include <tempo_utils/integer_types.h>

#include "base_ref.h"
#include "data_cell.h"

namespace lyric_runtime {

    tu_uint32 serialize_value(const DataCell &value, lyric_serde::PatchsetState &state);
}

#endif // LYRIC_RUNTIME_SERIALIZE_VALUE_H