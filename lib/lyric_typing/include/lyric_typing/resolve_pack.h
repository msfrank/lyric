#ifndef LYRIC_TYPING_RESOLVE_PACK_H
#define LYRIC_TYPING_RESOLVE_PACK_H

#include <lyric_assembler/abstract_resolver.h>
#include <lyric_assembler/assembly_state.h>

#include "typing_types.h"

namespace lyric_typing {

    tempo_utils::Result<lyric_assembler::ParameterPack> resolve_pack(
        const PackSpec &packSpec,
        lyric_assembler::AbstractResolver *resolver,
        lyric_assembler::AssemblyState *state);
}

#endif // LYRIC_TYPING_RESOLVE_PACK_H
