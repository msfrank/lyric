#ifndef LYRIC_REWRITER_INTERNAL_REWRITE_ARCHETYPE_H
#define LYRIC_REWRITER_INTERNAL_REWRITE_ARCHETYPE_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_parser/node_walker.h>
#include <lyric_object/lyric_object.h>
#include <lyric_rewriter/internal/entry_point.h>

namespace lyric_rewriter::internal {

    tempo_utils::Status
    rewrite_macro_list(
        lyric_parser::ArchetypeNode *blockNode,
        int index,
        lyric_parser::ArchetypeNode *macroListNode,
        lyric_rewriter::internal::EntryPoint &entryPoint);

    tempo_utils::Status
    rewrite_block(
        lyric_parser::ArchetypeNode *blockNode,
        EntryPoint &entryPoint);

    tempo_utils::Status
    rewrite_node(
        lyric_parser::ArchetypeNode *node,
        EntryPoint &entryPoint);

    tempo_utils::Result<lyric_parser::LyricArchetype>
    rewrite_root(
        lyric_parser::ArchetypeNode *root,
        EntryPoint &entryPoint);
}

#endif // LYRIC_REWRITER_INTERNAL_REWRITE_ARCHETYPE_H