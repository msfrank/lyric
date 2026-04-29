#ifndef LYRIC_TYPING_INTERNAL_COMPARE_CONCRETE_H
#define LYRIC_TYPING_INTERNAL_COMPARE_CONCRETE_H

#include <lyric_assembler/object_state.h>
#include <lyric_common/type_def.h>
#include <lyric_runtime/runtime_types.h>

namespace lyric_typing::internal {


    tempo_utils::Result<lyric_runtime::TypeComparison> compare_concrete_to_concept(
        const lyric_common::TypeDef &toConcept,
        const lyric_common::TypeDef &fromConcrete,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_concrete_to_concrete(
        const lyric_common::TypeDef &toConcrete,
        const lyric_common::TypeDef &fromConcrete,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_union_to_concrete(
        const lyric_common::TypeDef &toConcrete,
        const lyric_common::TypeDef &fromUnion,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_concrete(
        const lyric_common::TypeDef &toConcrete,
        const lyric_common::TypeDef &fromType,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_INTERNAL_COMPARE_CONCRETE_H
