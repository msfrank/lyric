#ifndef LYRIC_COMPILER_CONSTANT_UTILS_H
#define LYRIC_COMPILER_CONSTANT_UTILS_H

#include <lyric_assembler/block_handle.h>
#include <lyric_common/type_def.h>
#include <lyric_parser/archetype_node.h>

#include "compiler_scan_driver.h"

namespace lyric_compiler {

    tempo_utils::Status compile_nil(
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_undef(
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_true(
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_false(
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_char(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_integer(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_float(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_string(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_url(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_symbol(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        CompilerScanDriver *driver);
}

#endif // LYRIC_COMPILER_CONSTANT_UTILS_H
