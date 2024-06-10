#ifndef LYRIC_TYPING_RESOLVE_ASSIGNABLE_H
#define LYRIC_TYPING_RESOLVE_ASSIGNABLE_H

#include <lyric_assembler/assignable_type.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_parser/node_walker.h>

namespace lyric_typing {

    tempo_utils::Result<lyric_common::TypeDef> resolve_singular(
        const lyric_parser::Assignable &assignable,
        lyric_assembler::AbstractResolver *resolver,
        lyric_assembler::AssemblyState *state);

    tempo_utils::Result<lyric_common::TypeDef> resolve_assignable(
        const lyric_parser::Assignable &assignable,
        lyric_assembler::AbstractResolver *resolver,
        lyric_assembler::AssemblyState *state);
}

#endif // LYRIC_TYPING_RESOLVE_ASSIGNABLE_H