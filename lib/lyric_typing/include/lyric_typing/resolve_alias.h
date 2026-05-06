#ifndef LYRIC_TYPING_RESOLVE_ALIAS_H
#define LYRIC_TYPING_RESOLVE_ALIAS_H

#include <lyric_assembler/object_state.h>

namespace lyric_typing {

    tempo_utils::Result<lyric_common::TypeDef> resolve_impl_alias(
        const lyric_common::TypeDef &fromRef,
        const lyric_common::TypeDef &implType,
        int placeholderIndex,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_RESOLVE_ALIAS_H
