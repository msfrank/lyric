#ifndef LYRIC_COMPILER_COMPILE_IMPORT_H
#define LYRIC_COMPILER_COMPILE_IMPORT_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>

namespace lyric_compiler::internal {

    tempo_utils::Status compile_import_module(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Status compile_import_symbols(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Status compile_import_all(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_IMPORT_H
