#ifndef LYRIC_COMPILER_COMPILE_CONDITIONAL_H
#define LYRIC_COMPILER_COMPILE_CONDITIONAL_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/module_entry.h>

namespace lyric_compiler::internal {

    tempo_utils::Result<lyric_common::TypeDef> compile_cond(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Status compile_if(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_CONDITIONAL_H
