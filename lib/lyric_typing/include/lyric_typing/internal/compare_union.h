#ifndef LYRIC_TYPING_INTERNAL_COMPARE_UNION_H
#define LYRIC_TYPING_INTERNAL_COMPARE_UNION_H

#include <lyric_assembler/object_state.h>
#include <lyric_common/type_def.h>
#include <lyric_runtime/runtime_types.h>

namespace lyric_typing::internal {

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_union(
        const lyric_common::TypeDef &toUnion,
        const lyric_common::TypeDef &fromType,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_concrete_to_union(
        const lyric_common::TypeDef &toUnion,
        const lyric_common::TypeDef &fromConcrete,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_placeholder_to_union(
        const lyric_common::TypeDef &toUnion,
        const lyric_common::TypeDef &fromPlaceholder,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_union_to_union(
        const lyric_common::TypeDef &toUnion,
        const lyric_common::TypeDef &fromUnion,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_INTERNAL_COMPARE_UNION_H
