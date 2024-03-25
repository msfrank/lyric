#ifndef LYRIC_TYPING_COMPARE_ASSIGNABLE_H
#define LYRIC_TYPING_COMPARE_ASSIGNABLE_H

#include <lyric_assembler/assembly_state.h>

namespace lyric_typing {

    tempo_utils::Result<lyric_runtime::TypeComparison> compare_assignable(
        const lyric_common::TypeDef &toRef,
        const lyric_common::TypeDef &fromRef,
        lyric_assembler::AssemblyState *state);

    tempo_utils::Result<bool> is_implementable(
        const lyric_common::TypeDef &toConcept,
        const lyric_common::TypeDef &fromRef,
        lyric_assembler::AssemblyState *state);
}

#endif // LYRIC_TYPING_COMPARE_ASSIGNABLE_H