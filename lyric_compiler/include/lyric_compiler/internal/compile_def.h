#ifndef LYRIC_COMPILER_COMPILE_DEFINITION_H
#define LYRIC_COMPILER_COMPILE_DEFINITION_H

#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/module_entry.h>

namespace lyric_compiler::internal {

    tempo_utils::Status compile_def(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_DEFINITION_H
