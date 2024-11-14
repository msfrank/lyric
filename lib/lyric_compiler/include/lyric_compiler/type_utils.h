#ifndef LYRIC_COMPILER_TYPE_UTILS_H
#define LYRIC_COMPILER_TYPE_UTILS_H

#include <lyric_assembler/block_handle.h>
#include <lyric_common/type_def.h>
#include <lyric_typing/type_system.h>

#include "compiler_scan_driver.h"

namespace lyric_compiler {

    tempo_utils::Status match_types(
        const lyric_common::TypeDef &targetType,
        const lyric_common::TypeDef &matchType,
        lyric_assembler::BlockHandle *block,
        lyric_compiler::CompilerScanDriver *driver);

    tempo_utils::Status load_type(
        const lyric_parser::ArchetypeNode *node,
        lyric_assembler::CodeFragment *fragment,
        lyric_assembler::BlockHandle *block,
        lyric_compiler::CompilerScanDriver *driver);
}

#endif // LYRIC_COMPILER_TYPE_UTILS_H