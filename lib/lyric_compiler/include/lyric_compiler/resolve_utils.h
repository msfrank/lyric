#ifndef LYRIC_COMPILER_RESOLVE_UTILS_H
#define LYRIC_COMPILER_RESOLVE_UTILS_H

#include <lyric_assembler/block_handle.h>
#include <lyric_common/type_def.h>
#include <lyric_parser/archetype_node.h>

#include "compiler_scan_driver.h"
#include "data_deref_handler.h"

namespace lyric_compiler {

    tempo_utils::Status resolve_name(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::DataReference &ref);

    tempo_utils::Status resolve_member(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::DataReference &ref,
        const std::vector<DerefEffect> &effects,
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);
}

#endif // LYRIC_COMPILER_RESOLVE_UTILS_H
