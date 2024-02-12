#ifndef LYRIC_COMPILER_COMPILE_DEREF_H
#define LYRIC_COMPILER_COMPILE_DEREF_H

#include <lyric_assembler/block_handle.h>
#include <lyric_compiler/module_entry.h>

namespace lyric_compiler::internal {

    tempo_utils::Result<lyric_common::TypeDef> compile_deref(
        lyric_assembler::BlockHandle *block,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Result<lyric_common::TypeDef> compile_deref_this(
        lyric_assembler::BlockHandle *loadBlock,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Result<lyric_common::TypeDef> compile_deref_name(
        lyric_assembler::BlockHandle *bindingBlock,
        lyric_assembler::BlockHandle *loadBlock,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Result<lyric_common::TypeDef> compile_deref_call(
        lyric_assembler::BlockHandle *bindingBlock,
        lyric_assembler::BlockHandle *loadBlock,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

//    tempo_utils::Result<lyric_common::TypeDef> compile_deref_extension(
//        lyric_assembler::BlockHandle *block,
//        const std::string &identifier,
//        const lyric_common::TypeDef &receiverType,
//        const lyric_parser::NodeWalker &walker,
//        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Result<lyric_common::TypeDef> compile_deref_member(
        lyric_assembler::BlockHandle *block,
        const lyric_common::TypeDef &receiverType,
        bool isReceiver,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);

    tempo_utils::Result<lyric_common::TypeDef> compile_deref_method(
        lyric_assembler::BlockHandle *block,
        const lyric_common::TypeDef &receiverType,
        bool isReceiver,
        const lyric_parser::NodeWalker &walker,
        lyric_compiler::ModuleEntry &moduleEntry);
}

#endif // LYRIC_COMPILER_COMPILE_DEREF_H
