#ifndef LYRIC_COMPILER_DEREF_UTILS_H
#define LYRIC_COMPILER_DEREF_UTILS_H

#include <lyric_assembler/block_handle.h>
#include <lyric_common/type_def.h>
#include <lyric_parser/archetype_node.h>

#include "compiler_scan_driver.h"

namespace lyric_compiler {

    tempo_utils::Status deref_this(
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status deref_this(
        lyric_assembler::DataReference &ref,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status deref_name(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status deref_name(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::DataReference &ref,
        lyric_assembler::BlockHandle **blockptr,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status deref_namespace(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::DataReference &ref,
        lyric_assembler::BlockHandle **blockptr,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status deref_member(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::DataReference &ref,
        const lyric_common::TypeDef &receiverType,
        bool thisReceiver,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status deref_descriptor(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::BlockHandle **blockptr,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);
}

#endif // LYRIC_COMPILER_DEREF_UTILS_H
