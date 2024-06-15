#ifndef LYRIC_COMPILER_COMPILE_CALL_H
#define LYRIC_COMPILER_COMPILE_CALL_H

#include <lyric_assembler/abstract_placement.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_typing/callsite_reifier.h>

namespace lyric_compiler::internal {

    tempo_utils::Status compile_placement(
        const lyric_assembler::AbstractPlacement *placement,
        lyric_assembler::BlockHandle *bindingBlock,
        lyric_assembler::BlockHandle *invokeBlock,
        lyric_typing::CallsiteReifier &reifier,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Result<lyric_common::TypeDef> compile_function_call(
        lyric_assembler::BlockHandle *bindingBlock,
        lyric_assembler::BlockHandle *invokeBlock,
        const std::string &functionName,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_CALL_H
