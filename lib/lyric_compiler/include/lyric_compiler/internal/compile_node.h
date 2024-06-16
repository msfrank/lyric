#ifndef LYRIC_COMPILER_COMPILE_NODE_H
#define LYRIC_COMPILER_COMPILE_NODE_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_parser/node_walker.h>

namespace lyric_compiler::internal {

    tempo_utils::Result<lyric_common::TypeDef> compile_expression(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Status compile_statement(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Result<lyric_common::TypeDef> compile_node(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_NODE_H
