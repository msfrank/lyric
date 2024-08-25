#ifndef LYRIC_COMPILER_DEREF_UTILS_H
#define LYRIC_COMPILER_DEREF_UTILS_H

#include <lyric_assembler/block_handle.h>
#include <lyric_common/type_def.h>
#include <lyric_parser/archetype_node.h>

#include "compiler_scan_driver.h"

namespace lyric_compiler {

    tempo_utils::Status compile_this(
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_name(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);
}

#endif // LYRIC_COMPILER_DEREF_UTILS_H
