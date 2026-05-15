#ifndef LYRIC_TYPING_RESOLVE_IMPLEMENTABLE_H
#define LYRIC_TYPING_RESOLVE_IMPLEMENTABLE_H

#include <lyric_assembler/block_handle.h>
#include <lyric_parser/node_walker.h>

#include "type_spec.h"

namespace lyric_typing {

    tempo_utils::Result<lyric_common::TypeDef> resolve_implementable(
        const TypeSpec &typeSpec,
        lyric_assembler::AbstractResolver *resolver,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_RESOLVE_IMPLEMENTABLE_H
