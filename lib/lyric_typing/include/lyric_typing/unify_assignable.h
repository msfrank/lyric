#ifndef LYRIC_TYPING_UNIFY_ASSIGNABLE_H
#define LYRIC_TYPING_UNIFY_ASSIGNABLE_H

#include <lyric_assembler/object_state.h>

namespace lyric_typing {

    tempo_utils::Result<lyric_common::TypeDef> unify_assignable(
        const lyric_common::TypeDef &ref1,
        const lyric_common::TypeDef &ref2,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_UNIFY_ASSIGNABLE_H