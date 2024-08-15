#ifndef LYRIC_ANALYZER_ANALYZE_MODULE_H
#define LYRIC_ANALYZER_ANALYZE_MODULE_H

#include <lyric_assembler/object_state.h>
#include <lyric_object/lyric_object.h>

#include "entry_point.h"

namespace lyric_analyzer::internal {

    tempo_utils::Result<lyric_object::LyricObject> analyze_module(
        const lyric_parser::NodeWalker &walker,
        EntryPoint &entryPoint);
}

#endif // LYRIC_ANALYZER_ANALYZE_MODULE_H