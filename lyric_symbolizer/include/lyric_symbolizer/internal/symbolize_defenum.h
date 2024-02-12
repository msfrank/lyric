#ifndef LYRIC_SYMBOLIZER_SYMBOLIZE_DEFENUM_H
#define LYRIC_SYMBOLIZER_SYMBOLIZE_DEFENUM_H

#include <lyric_parser/node_walker.h>
#include <lyric_symbolizer/internal/entry_point.h>
#include <lyric_symbolizer/internal/symbolize_handle.h>

namespace lyric_symbolizer::internal {

    tempo_utils::Status
    symbolize_defenum(
        SymbolizeHandle *block,
        const lyric_parser::NodeWalker &walker,
        EntryPoint &entryPoint);
}

#endif // LYRIC_SYMBOLIZER_SYMBOLIZE_DEFENUM_H