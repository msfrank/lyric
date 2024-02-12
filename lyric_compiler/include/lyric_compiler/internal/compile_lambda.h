#ifndef LYRIC_COMPILER_COMPILE_LAMBDA_H
#define LYRIC_COMPILER_COMPILE_LAMBDA_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>

namespace lyric_compiler::internal {

    tempo_utils::Result<lyric_common::TypeDef> compile_lambda(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry,
        const std::vector<lyric_object::TemplateParameter> &templateParameters = {});
}

#endif // LYRIC_COMPILER_COMPILE_LAMBDA_H
