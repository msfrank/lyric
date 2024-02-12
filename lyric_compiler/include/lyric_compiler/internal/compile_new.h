#ifndef LYRIC_COMPILER_COMPILE_NEW_H
#define LYRIC_COMPILER_COMPILE_NEW_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/interpreter_state.h>

namespace lyric_compiler::internal {

    tempo_utils::Result<lyric_common::TypeDef> compile_new(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry,
        const lyric_parser::Assignable &typeHint = {});
}

#endif // LYRIC_COMPILER_COMPILE_NEW_H
