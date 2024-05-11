#ifndef LYRIC_COMPILER_COMPILE_UNWRAP_H
#define LYRIC_COMPILER_COMPILE_UNWRAP_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>

namespace lyric_compiler::internal {

    constexpr char const *kUnwrapExtensionName = "unwrap";

    tempo_utils::Status compile_unwrap(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        const lyric_common::TypeDef &unwrapType,
        const lyric_assembler::DataReference &targetRef,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_UNWRAP_H
