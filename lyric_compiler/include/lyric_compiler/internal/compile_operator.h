#ifndef LYRIC_COMPILER_COMPILE_OPERATOR_H
#define LYRIC_COMPILER_COMPILE_OPERATOR_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_schema/ast_schema.h>

namespace lyric_compiler::internal {

    tempo_utils::Status match_types(
        const lyric_common::TypeDef &targetType,
        const lyric_common::TypeDef &matchType,
        const lyric_parser::NodeWalker &walker,
        lyric_assembler::BlockHandle *block,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Result<lyric_common::TypeDef> compile_operator_call(
        lyric_assembler::BlockHandle *block,
        lyric_schema::LyricAstId operationId,
        const std::vector<lyric_common::TypeDef> &argList,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Result<lyric_common::TypeDef> compile_operator(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_OPERATOR_H
