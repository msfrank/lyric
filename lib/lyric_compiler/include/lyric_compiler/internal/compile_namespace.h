#ifndef LYRIC_COMPILER_COMPILE_NAMESPACE_H
#define LYRIC_COMPILER_COMPILE_NAMESPACE_H

#include <lyric_assembler/assignable_type.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_parser/node_walker.h>

namespace lyric_compiler::internal {

    tempo_utils::Status compile_namespace(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_NAMESPACE_H
