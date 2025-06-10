#ifndef LYRIC_TYPING_PARSE_ASSIGNABLE_H
#define LYRIC_TYPING_PARSE_ASSIGNABLE_H

#include <lyric_assembler/block_handle.h>
#include <lyric_parser/node_walker.h>
#include <tempo_utils/result.h>

#include "type_spec.h"

namespace lyric_typing {

    tempo_utils::Result<TypeSpec>
    parse_assignable(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker);

    tempo_utils::Result<std::vector<TypeSpec>>
    parse_type_arguments(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker);

}

#endif // LYRIC_TYPING_PARSE_ASSIGNABLE_H