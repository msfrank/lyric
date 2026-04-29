#ifndef LYRIC_TYPING_INTERNAL_COMPARE_INTERSECTION_H
#define LYRIC_TYPING_INTERNAL_COMPARE_INTERSECTION_H

#include <lyric_assembler/object_state.h>
#include <lyric_common/type_def.h>
#include <lyric_runtime/runtime_types.h>

namespace lyric_typing::internal {

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_intersection(
        const lyric_common::TypeDef &toIntersection,
        const lyric_common::TypeDef &fromType,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_intersection_to_intersection(
        const lyric_common::TypeDef &toIntersection,
        const lyric_common::TypeDef &fromIntersection,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_concrete_to_intersection(
        const lyric_common::TypeDef &toIntersection,
        const lyric_common::TypeDef &fromConcrete,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_INTERNAL_COMPARE_INTERSECTION_H
