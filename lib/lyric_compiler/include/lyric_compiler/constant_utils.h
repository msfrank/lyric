#ifndef LYRIC_COMPILER_CONSTANT_UTILS_H
#define LYRIC_COMPILER_CONSTANT_UTILS_H

#include <lyric_assembler/block_handle.h>
#include <lyric_common/type_def.h>
#include <lyric_parser/archetype_node.h>

#include "compiler_scan_driver.h"

namespace lyric_compiler {

    tempo_utils::Status constant_nil(
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status constant_undef(
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status constant_true(
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status constant_false(
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status constant_char(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status constant_integer(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status constant_float(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status constant_string(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status constant_url(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);
}

#endif // LYRIC_COMPILER_CONSTANT_UTILS_H
