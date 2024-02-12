#ifndef LYRIC_ANALYZER_ANALYZE_NODE_H
#define LYRIC_ANALYZER_ANALYZE_NODE_H

#include <lyric_assembler/block_handle.h>

#include "entry_point.h"

namespace lyric_analyzer::internal {

    tempo_utils::Status analyze_val(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        EntryPoint &entryPoint);

    tempo_utils::Status analyze_var(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        EntryPoint &entryPoint);

    tempo_utils::Status analyze_block(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        EntryPoint &entryPoint);

    tempo_utils::Status analyze_node(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        EntryPoint &entryPoint);
}

#endif // LYRIC_ANALYZER_ANALYZE_NODE_H