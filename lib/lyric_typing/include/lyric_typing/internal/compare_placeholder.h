#ifndef LYRIC_TYPING_INTERNAL_COMPARE_PLACEHOLDER_H
#define LYRIC_TYPING_INTERNAL_COMPARE_PLACEHOLDER_H

#include <lyric_assembler/object_state.h>
#include <lyric_common/type_def.h>
#include <lyric_runtime/runtime_types.h>

namespace lyric_typing::internal {

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_placeholder(
        const lyric_common::TypeDef &toPlaceholder,
        const lyric_common::TypeDef &fromType,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_placeholder_to_placeholder(
        const lyric_common::TypeDef &toPlaceholder,
        const lyric_common::TypeDef &fromPlaceholder,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_INTERNAL_COMPARE_PLACEHOLDER_H
