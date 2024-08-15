#ifndef LYRIC_SYMBOLIZER_SYMBOLIZE_MODULE_H
#define LYRIC_SYMBOLIZER_SYMBOLIZE_MODULE_H

#include <lyric_assembler/object_state.h>
#include <lyric_parser/node_walker.h>
#include <lyric_object/lyric_object.h>
#include <lyric_symbolizer/internal/entry_point.h>
#include <lyric_symbolizer/internal/symbolize_handle.h>

namespace lyric_symbolizer::internal {

    tempo_utils::Status
    symbolize_block(
        SymbolizeHandle *block,
        const lyric_parser::NodeWalker &walker,
        EntryPoint &entryPoint);

    tempo_utils::Status
    symbolize_node(
        SymbolizeHandle *block,
        const lyric_parser::NodeWalker &walker,
        EntryPoint &entryPoint);

    tempo_utils::Result<lyric_object::LyricObject>
    symbolize_module(
        const lyric_parser::NodeWalker &walker,
        EntryPoint &entryPoint);
}

#endif // LYRIC_SYMBOLIZER_SYMBOLIZE_MODULE_H
