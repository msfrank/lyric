#ifndef LYRIC_COMPILER_COMPILE_DEFENUM_H
#define LYRIC_COMPILER_COMPILE_DEFENUM_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>

namespace lyric_compiler::internal {

    tempo_utils::Status compile_defenum(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry,
        lyric_assembler::EnumSymbol **enumSymbolPtr = nullptr);
}

#endif // LYRIC_COMPILER_COMPILE_DEFENUM_H