#ifndef LYRIC_TYPING_PARSE_PACK_H
#define LYRIC_TYPING_PARSE_PACK_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_parser/node_walker.h>

namespace lyric_typing {

    tempo_utils::Result<lyric_assembler::PackSpec>
    parse_pack(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_assembler::AssemblyState *state);

}

#endif // LYRIC_TYPING_PARSE_PACK_H