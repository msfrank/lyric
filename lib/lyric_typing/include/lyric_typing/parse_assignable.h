#ifndef LYRIC_TYPING_PARSE_ASSIGNABLE_H
#define LYRIC_TYPING_PARSE_ASSIGNABLE_H

#include "lyric_parser/assignable.h"
#include "tempo_utils/result.h"
#include "lyric_assembler/block_handle.h"
#include "lyric_parser/node_walker.h"

namespace lyric_typing {

    tempo_utils::Result<lyric_parser::TypeSpec>
    parse_assignable(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_assembler::AssemblyState *state);

}

#endif // LYRIC_TYPING_PARSE_ASSIGNABLE_H