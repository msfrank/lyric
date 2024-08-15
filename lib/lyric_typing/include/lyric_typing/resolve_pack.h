#ifndef LYRIC_TYPING_RESOLVE_PACK_H
#define LYRIC_TYPING_RESOLVE_PACK_H

#include <lyric_assembler/abstract_resolver.h>
#include <lyric_assembler/object_state.h>

#include "typing_types.h"

namespace lyric_typing {

    tempo_utils::Result<lyric_assembler::ParameterPack> resolve_pack(
        const PackSpec &packSpec,
        lyric_assembler::AbstractResolver *resolver,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_RESOLVE_PACK_H
