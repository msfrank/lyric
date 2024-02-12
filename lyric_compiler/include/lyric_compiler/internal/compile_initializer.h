#ifndef LYRIC_COMPILER_COMPILE_INITIALIZER_H
#define LYRIC_COMPILER_COMPILE_INITIALIZER_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>

namespace lyric_compiler::internal {

    tempo_utils::Result<lyric_common::SymbolUrl>
    compile_default_initializer(
        lyric_assembler::BlockHandle *block,
        const std::string &name,
        const lyric_parser::Assignable &type,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Status
    compile_static_initializer(
        lyric_assembler::StaticSymbol *staticSymbol,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_INITIALIZER_H