#ifndef LYRIC_TYPING_RESOLVE_ASSIGNABLE_H
#define LYRIC_TYPING_RESOLVE_ASSIGNABLE_H

#include <lyric_assembler/block_handle.h>
#include <lyric_parser/node_walker.h>

#include "type_spec.h"

namespace lyric_typing {

    tempo_utils::Result<lyric_common::TypeDef> resolve_singular(
        const TypeSpec &typeSpec,
        lyric_assembler::AbstractResolver *resolver,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<lyric_common::TypeDef> resolve_assignable(
        const TypeSpec &typeSpec,
        lyric_assembler::AbstractResolver *resolver,
        lyric_assembler::ObjectState *state);

    tempo_utils::Result<std::vector<lyric_common::TypeDef>> resolve_type_arguments(
        const std::vector<TypeSpec> &typeArgumentsSpec,
        lyric_assembler::AbstractResolver *resolver,
        lyric_assembler::ObjectState *state);
}

#endif // LYRIC_TYPING_RESOLVE_ASSIGNABLE_H