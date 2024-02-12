#ifndef LYRIC_TYPING_RESOLVE_ASSIGNABLE_H
#define LYRIC_TYPING_RESOLVE_ASSIGNABLE_H

#include <lyric_assembler/assignable_type.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_parser/node_walker.h>

namespace lyric_typing {

    tempo_utils::Result<lyric_common::TypeDef> resolve_S_or_P_type(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_assembler::AssemblyState *state);

    tempo_utils::Result<lyric_common::TypeDef> resolve_assignable(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_assembler::AssemblyState *state);
}

#endif // LYRIC_TYPING_RESOLVE_ASSIGNABLE_H