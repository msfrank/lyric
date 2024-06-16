
#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_block.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_block(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT(walker.isValid());
    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstBlockClass, 1);

    auto *code = block->blockCode();

    lyric_common::TypeDef resultType;

    for (int i = 0, last = walker.numChildren() - 1; i <= last; i++) {
        auto compileNodeResult = compile_node(block, walker.getChild(i), moduleEntry);
        if (compileNodeResult.isStatus())
            return compileNodeResult;
        resultType = compileNodeResult.getResult();
        if (!resultType.isValid())
            block->throwSyntaxError(walker.getChild(0),
                "block form returns an invalid result");
        // discard intermediate expression result
        if (i < last && resultType.getType() != lyric_common::TypeDefType::NoReturn) {
            code->popValue();
        }
    }

    return resultType;
}

tempo_utils::Status
lyric_compiler::internal::compile_proc_block(
    lyric_assembler::CallSymbol *callSymbol,
    const lyric_assembler::ParameterPack &parameterPack,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT(walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstBlockClass, 1);

    // define the function
    lyric_assembler::ProcHandle *proc;
    TU_ASSIGN_OR_RETURN (proc, callSymbol->defineCall(parameterPack));

    auto *block = proc->procBlock();
    auto *code = proc->procCode();

    // compile the block and determine the body type
    lyric_common::TypeDef bodyType;
    TU_ASSIGN_OR_RETURN (bodyType, compile_block(block, walker, moduleEntry));

    // add return instruction
    TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_RETURN));

    // finalize the call
    TU_RETURN_IF_NOT_OK (callSymbol->finalizeCall());

    auto returnType = callSymbol->getReturnType();
    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, bodyType));
    if (!isReturnable)
        return block->logAndContinue(walker,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", returnType.toString());

    // validate that each exit returns the expected type
    for (auto it = proc->exitTypesBegin(); it != proc->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, *it));
        if (!isReturnable)
            return block->logAndContinue(walker,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", returnType.toString());
    }

    return {};
}

tempo_utils::Status
lyric_compiler::internal::compile_proc_block(
    lyric_assembler::CallSymbol *callSymbol,
    const lyric_assembler::ParameterPack &parameterPack,
    const lyric_common::TypeDef &returnType,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT(walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstBlockClass, 1);

    // define the function
    lyric_assembler::ProcHandle *proc;
    TU_ASSIGN_OR_RETURN (proc, callSymbol->defineCall(parameterPack, returnType));

    auto *block = proc->procBlock();
    auto *code = proc->procCode();

    // compile the block and determine the body type
    lyric_common::TypeDef bodyType;
    TU_ASSIGN_OR_RETURN (bodyType, compile_block(block, walker, moduleEntry));

    // add return instruction
    TU_RETURN_IF_NOT_OK (code->writeOpcode(lyric_object::Opcode::OP_RETURN));

    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, bodyType));
    if (!isReturnable)
        return block->logAndContinue(walker,
            lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "body does not match return type {}", returnType.toString());

    // validate that each exit returns the expected type
    for (auto it = proc->exitTypesBegin(); it != proc->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(returnType, *it));
        if (!isReturnable)
            return block->logAndContinue(walker,
                lyric_compiler::CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "body does not match return type {}", returnType.toString());
    }

    return {};
}
