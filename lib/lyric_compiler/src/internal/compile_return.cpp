
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_return.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_schema/ast_schema.h>

tempo_utils::Status
lyric_compiler::internal::compile_return(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    moduleEntry.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstReturnClass, 1);

    lyric_common::TypeDef exitType;
    TU_ASSIGN_OR_RETURN (exitType, compile_expression(block, walker.getChild(0), moduleEntry));

    auto *procHandle = block->blockProc();
    procHandle->putExitType(exitType);

    return {};
}