#ifndef LYRIC_COMPILER_RESOLVE_UTILS_H
#define LYRIC_COMPILER_RESOLVE_UTILS_H

#include <lyric_assembler/block_handle.h>
#include <lyric_common/type_def.h>
#include <lyric_parser/archetype_node.h>

#include "compiler_scan_driver.h"

namespace lyric_compiler {

    tempo_utils::Status resolve_name(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::DataReference &ref,
        CompilerScanDriver *driver);

    tempo_utils::Status resolve_member(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        const lyric_common::TypeDef &receiverType,
        bool thisReceiver,
        lyric_assembler::DataReference &ref,
        CompilerScanDriver *driver);
}

#endif // LYRIC_COMPILER_RESOLVE_UTILS_H
