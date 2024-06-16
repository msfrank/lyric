#ifndef LYRIC_COMPILER_COMPILE_BLOCK_H
#define LYRIC_COMPILER_COMPILE_BLOCK_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_parser/node_walker.h>

namespace lyric_compiler::internal {

    tempo_utils::Result<lyric_common::TypeDef> compile_block(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Status compile_proc_block(
        lyric_assembler::CallSymbol *callSymbol,
        const lyric_assembler::ParameterPack &parameterPack,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Status compile_proc_block(
        lyric_assembler::CallSymbol *callSymbol,
        const lyric_assembler::ParameterPack &parameterPack,
        const lyric_common::TypeDef &returnType,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_BLOCK_H
