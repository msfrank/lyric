
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/impl_utils.h>
#include <lyric_compiler/operator_utils.h>
#include <lyric_typing/callsite_reifier.h>

tempo_utils::Status
lyric_compiler::compile_unary_operator(
    lyric_schema::LyricAstId operationId,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (driver != nullptr);

    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *symbolCache = state->symbolCache();
    auto *typeSystem = driver->getTypeSystem();

    lyric_common::TypeDef operandType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    lyric_common::TypeDef operatorType;
    TU_ASSIGN_OR_RETURN (operatorType, resolve_unary_operator_concept_type(
        fundamentalCache, operationId, operandType));

    std::string actionName;
    TU_ASSIGN_OR_RETURN (actionName, resolve_operator_action_name(operationId));

    // resolve operator impl reference
    lyric_assembler::ImplReference implRef;
    TU_ASSIGN_OR_RETURN (implRef, block->resolveImpl(operatorType));

    lyric_assembler::CallableInvoker extensionInvoker;
    TU_RETURN_IF_NOT_OK (prepare_impl_action(actionName, implRef, extensionInvoker, block, symbolCache));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(extensionInvoker));

    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(operandType));

    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, extensionInvoker.invoke(block, reifier, fragment));

    return driver->pushResult(resultType);
}

tempo_utils::Status
lyric_compiler::compile_binary_operator(
    lyric_schema::LyricAstId operationId,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (driver != nullptr);

    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *symbolCache = state->symbolCache();
    auto *typeSystem = driver->getTypeSystem();

    lyric_common::TypeDef rhsType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    lyric_common::TypeDef lhsType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    lyric_common::TypeDef operatorType;
    TU_ASSIGN_OR_RETURN (operatorType, resolve_binary_operator_concept_type(
        fundamentalCache, operationId, lhsType, rhsType));

    std::string actionName;
    TU_ASSIGN_OR_RETURN (actionName, resolve_operator_action_name(operationId));

    // resolve operator impl reference
    lyric_assembler::ImplReference implRef;
    TU_ASSIGN_OR_RETURN (implRef, block->resolveImpl(operatorType));

    lyric_assembler::CallableInvoker extensionInvoker;
    TU_RETURN_IF_NOT_OK (prepare_impl_action(actionName, implRef, extensionInvoker, block, symbolCache));

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(extensionInvoker));

    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(lhsType));
    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(rhsType));

    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, extensionInvoker.invoke(block, reifier, fragment));

    return driver->pushResult(resultType);
}
