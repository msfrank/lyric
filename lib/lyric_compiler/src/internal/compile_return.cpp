
#include <lyric_assembler/call_symbol.h>
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

    auto compileExpressionResult = compile_expression(block, walker.getChild(0), moduleEntry);
    if (compileExpressionResult.isStatus())
        return compileExpressionResult.getStatus();
    auto exitType = compileExpressionResult.getResult();

    auto activationUrl = block->blockProc()->getActivation();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, block->blockState()->symbolCache()->getOrImportSymbol(activationUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
        block->throwAssemblerInvariant("invalid call symbol {}", activationUrl.toString());

    cast_symbol_to_call(symbol)->putExitType(exitType);

    return CompilerStatus::ok();
}