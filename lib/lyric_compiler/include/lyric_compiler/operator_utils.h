#ifndef LYRIC_COMPILER_OPERATOR_UTILS_H
#define LYRIC_COMPILER_OPERATOR_UTILS_H

#include <lyric_assembler/block_handle.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_schema/ast_schema.h>

#include "compiler_scan_driver.h"

namespace lyric_compiler {

    tempo_utils::Status compile_unary_operator(
        lyric_schema::LyricAstId operationId,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);

    tempo_utils::Status compile_binary_operator(
        lyric_schema::LyricAstId operationId,
        lyric_assembler::BlockHandle *block,
        lyric_assembler::CodeFragment *fragment,
        CompilerScanDriver *driver);
}
#endif // LYRIC_COMPILER_OPERATOR_UTILS_H
