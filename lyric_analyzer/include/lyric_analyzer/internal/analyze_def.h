#ifndef LYRIC_ANALYZER_ANALYZE_DEF_H
#define LYRIC_ANALYZER_ANALYZE_DEF_H

#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/template_handle.h>

#include "entry_point.h"

namespace lyric_analyzer::internal {

    tempo_utils::Status analyze_def(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        EntryPoint &entryPoint);
}

#endif // LYRIC_ANALYZER_ANALYZE_DEF_H
